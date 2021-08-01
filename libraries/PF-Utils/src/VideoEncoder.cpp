#include <pf_utils/VideoEncoder.hpp>

#include <filesystem>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <memory>
#include <array>
#include <fstream>
#include <cstdio>
#include <cstddef>
#include <exception>
#include <cstring>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
#include <libavutil/time.h>
#include <libavutil/pixfmt.h>
}

#include <fmt/format.h>
#include <gsl/util>

#include <pf_utils/FileUtils.hpp>

// TODO(poppyfanboy) Add a namespace block, remove redundant pf::util::...

std::string const pf::util::VideoEncoder::DEFAULT_CODEC_NAME = "libx264";
std::string const pf::util::VideoEncoder::DEFAULT_OUTPUT_FILE_EXTENSION = ".mp4";
std::string const pf::util::VideoEncoder::DEFAULT_OUTPUT_FILE_NAME = "Video";

pf::util::VideoEncoder::VideoEncoder(std::filesystem::path filePath,
                                     size_t width,
                                     size_t height,
                                     size_t fps,
                                     uint64_t bitrate)
    : _filePath(std::move(filePath))
    , _width(width)
    , _height(height)
    , _fps(std::max(fps, static_cast<size_t>(1)))
    , _bitrate(bitrate)
    , _swsContext(nullptr, nullptr)
    , _frame(nullptr, nullptr)
{
}

pf::util::VideoEncoder::VideoEncoder()
    : VideoEncoder(std::filesystem::path(), 1, 1, 1, 1000)
{
}

pf::util::VideoEncoder::~VideoEncoder()
{
    if (started() && !finished())
    {
        finish();
    }
}

// TODO(poppyfanboy) Break this function into smaller ones
void pf::util::VideoEncoder::start()
{
    // TODO(poppyfanboy) Add a way to connect a logger to the encoder and redirect AV lib logs
    av_log_set_level(AV_LOG_QUIET);

    // * Guess output format, create encoder and encoder context *

    _filePath = fixOutputFilePath(_filePath);
    _output.format = Output::guessFormat(_filePath);

    _encoder.codec = avcodec_find_encoder(_output.format->video_codec);
    if (_encoder.codec == nullptr)
    {
        // Use the default codec instead
        _filePath.replace_extension(DEFAULT_OUTPUT_FILE_EXTENSION);
        _output.format = Output::guessFormat(_filePath);

        _encoder.codec = avcodec_find_encoder(_output.format->video_codec);
        if (_encoder.codec == nullptr)
        {
            throw std::runtime_error("Failed to find both fallback and user specified codecs.");
        }
    }

    _encoder.context = UniquePointer<AVCodecContext>(
        avcodec_alloc_context3(_encoder.codec),
        [](AVCodecContext *encoderContext) { avcodec_free_context(&encoderContext); });
    if (_encoder.context == nullptr)
    {
        throw std::runtime_error("Failed to allocate encoder context");
    }


    // * Create output context *

    _output.context = UniquePointer<AVFormatContext *>(new AVFormatContext *,
                                                       [](AVFormatContext **outputContext) {
                                                           avformat_free_context(*outputContext);
                                                           delete outputContext;
                                                       });
    int outputContextAllocResponse = avformat_alloc_output_context2(
        _output.context.get(), _output.format, nullptr, _filePath.string().c_str());
    if (outputContextAllocResponse != 0)
    {
        throw std::runtime_error("Failed to allocate output context.");
    }


    // * Create and configure output stream, configure encoder context *

    _output.stream = avformat_new_stream(*_output.context, _encoder.codec);
    if (_output.stream == nullptr)
    {
        throw std::runtime_error("Failed to create an output stream.");
    }

    _output.stream->codecpar->codec_id = _output.format->video_codec;
    _output.stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    _output.stream->codecpar->width = gsl::narrow_cast<int>(_width);
    _output.stream->codecpar->height = gsl::narrow_cast<int>(_height);
    _output.stream->codecpar->format = AV_PIX_FMT_YUV420P;
    _output.stream->codecpar->bit_rate = gsl::narrow_cast<int64_t>(_bitrate);
    if (_output.stream->codecpar->codec_id == AV_CODEC_ID_H264 ||
        _output.stream->codecpar->codec_id == AV_CODEC_ID_H265)
    {
        av_opt_set(_encoder.context->priv_data, "preset", "slow", 0);
    }

    _parametersSetter(*_encoder.context, *_output.stream);
    avcodec_parameters_to_context(_encoder.context.get(), _output.stream->codecpar);

    _encoder.context->time_base = {1, static_cast<int>(_fps)};
    _encoder.context->framerate = {static_cast<int>(_fps), 1};
    _encoder.context->gop_size = 10;
    _encoder.context->max_b_frames = 1;

    _parametersSetter(*_encoder.context, *_output.stream);
    avcodec_parameters_from_context(_output.stream->codecpar, _encoder.context.get());


    // * Open codec using encoder context *

    int openCodecResponse = avcodec_open2(_encoder.context.get(), _encoder.codec, nullptr);
    if (openCodecResponse < 0)
    {
        std::array<char, 64> responseString = {0};
        throw std::runtime_error("Failed to open codec " +
                                 std::string(av_make_error_string(responseString.data(),
                                                                  responseString.size(),
                                                                  openCodecResponse)));
    }


    // * Open file *

    if ((_output.format->flags & AVFMT_NOFILE) == 0)
    {
        int fileOpenResponse =
            avio_open(&(*_output.context)->pb, _filePath.string().c_str(), AVIO_FLAG_WRITE);
        if (fileOpenResponse < 0)
        {
            throw std::runtime_error("Failed to open file " + _filePath.string());
        }
    }


    // * Write header *

    int writeHeaderResponse = avformat_write_header(*_output.context, nullptr);
    if (writeHeaderResponse < 0)
    {
        throw std::runtime_error("Failed to write file header.");
    }
    av_dump_format(*_output.context, 0, _filePath.string().c_str(), 1);


    // * Create frame *

    _frame =
        UniquePointer<AVFrame>(av_frame_alloc(), [](AVFrame *frame) { av_frame_free(&frame); });
    if (_frame == nullptr)
    {
        throw std::runtime_error("Failed to allocate a video frame.");
    }
    _frame->format = _encoder.context->pix_fmt;
    _frame->width = _encoder.context->width;
    _frame->height = _encoder.context->height;
    _frame->pts = 0;

    int getBufferResponse = av_frame_get_buffer(_frame.get(), 0);
    if (getBufferResponse < 0)
    {
        throw std::runtime_error("Failed to allocate a buffer");
    }


    // * Create source image *

    _source.data = UniquePointer<uint8_t *>(new uint8_t *[4], [](uint8_t **data) {
        av_freep(&data[0]);
        delete[] data;
    });
    AVPixelFormat sourcePixelFormat = AV_PIX_FMT_RGB24;
    int sourceImageAllocResponse = av_image_alloc(_source.data.get(),
                                                  _source.lineSize.data(),
                                                  gsl::narrow_cast<int>(_width),
                                                  gsl::narrow_cast<int>(_height),
                                                  sourcePixelFormat,
                                                  1);
    if (sourceImageAllocResponse < 0)
    {
        throw std::runtime_error("Could not allocate source image");
    }


    // * Create destination image *

    _destination.data = UniquePointer<uint8_t *>(new uint8_t *[4], [](uint8_t **data) {
        av_freep(&data[0]);
        delete[] data;
    });
    auto destinationPixelFormat = static_cast<AVPixelFormat>(_output.stream->codecpar->format);
    int destinationImageAllocResponse = av_image_alloc(_destination.data.get(),
                                                       _destination.lineSize.data(),
                                                       gsl::narrow_cast<int>(_width),
                                                       gsl::narrow_cast<int>(_height),
                                                       destinationPixelFormat,
                                                       4);
    if (destinationImageAllocResponse < 0)
    {
        throw std::runtime_error("Could not allocate destination image");
    }


    // * Create SWS context *

    _swsContext =
        UniquePointer<SwsContext>(sws_getContext(gsl::narrow_cast<int>(_width),
                                                 gsl::narrow_cast<int>(_height),
                                                 sourcePixelFormat,
                                                 gsl::narrow_cast<int>(_width),
                                                 gsl::narrow_cast<int>(_height),
                                                 destinationPixelFormat,
                                                 SWS_BILINEAR,
                                                 nullptr,
                                                 nullptr,
                                                 nullptr),
                                  [](SwsContext *swsContext) { sws_freeContext(swsContext); });
    if (_swsContext == nullptr)
    {
        throw std::runtime_error("Failed to create the scale context.");
    }

    _started = true;
}

void pf::util::VideoEncoder::appendFrameFromRGB(std::vector<uint8_t> const &rgbData)
{
    if (_finished)
    {
        throw std::runtime_error("Cannot add a new frame after encoding has ended.");
    }
    if (!_started)
    {
        start();
    }
    if (rgbData.size() != 3 * _width * _height)
    {
        throw std::runtime_error(
            "Amount of the passed RGB bytes does not match with the dimensions of the video");
    }

    // Flip vertically before converting to YCbCr
    yFlippedImageCopy(reinterpret_cast<RGB *>(_source.data.get()[0]),
                      reinterpret_cast<RGB const *>(rgbData.data()),
                      _width,
                      _height);

    int makeWritableResponse = av_frame_make_writable(_frame.get());
    if (makeWritableResponse < 0)
    {
        throw std::runtime_error("Frame is not writable.");
    }
    sws_scale(_swsContext.get(),
              _source.data.get(),
              _source.lineSize.data(),
              0,
              gsl::narrow_cast<int>(_height),
              _destination.data.get(),
              _destination.lineSize.data());

    linewiseImageCopy(_frame->data[0],
                      _frame->linesize[0],
                      _destination.data.get()[0],
                      _destination.lineSize[0],
                      _width,
                      _height);

    if (_output.stream->codecpar->format == AV_PIX_FMT_YUV420P)
    {
        linewiseImageCopy(_frame->data[1],
                          _frame->linesize[1],
                          _destination.data.get()[1],
                          _destination.lineSize[1],
                          _width / 2,
                          _height / 2);

        linewiseImageCopy(_frame->data[2],
                          _frame->linesize[2],
                          _destination.data.get()[2],
                          _destination.lineSize[2],
                          _width / 2,
                          _height / 2);
    }

    _frame->pts = gsl::narrow_cast<int64_t>(_frameIndex) * _output.stream->time_base.den /
                  (_output.stream->time_base.num * gsl::narrow_cast<int64_t>(_fps));
    encodeFrame(*_encoder.context, *_frame);
    _frameIndex++;
}

void pf::util::VideoEncoder::finish()
{
    if (finished() || !started())
    {
        throw std::runtime_error(
            "Either the encoding has already ended or it has not started yet.");
    }
    _finished = true;

    AVPacket packet = {nullptr};
    av_init_packet(&packet);

    // encode delayed frames
    while (true)
    {
        avcodec_send_frame(_encoder.context.get(), nullptr);
        if (avcodec_receive_packet(_encoder.context.get(), &packet) == 0)
        {
            av_interleaved_write_frame(*_output.context, &packet);
            av_packet_unref(&packet);
        }
        else
        {
            break;
        }
    }

    av_write_trailer(*_output.context);
    if ((_output.format->flags & AVFMT_NOFILE) == 0)
    {
        int fileCloseResponse = avio_close((*_output.context)->pb);
        if (fileCloseResponse != 0)
        {
            throw std::runtime_error("Failed to close file " + _filePath.string());
        }
    }
}

bool pf::util::VideoEncoder::started() const
{
    return _started;
}

bool pf::util::VideoEncoder::finished() const
{
    return _finished;
}

size_t pf::util::VideoEncoder::framesCount() const
{
    return _frameIndex;
}

void pf::util::VideoEncoder::encodeFrame(AVCodecContext &encoderContext, AVFrame &frame)
{
    int sendFrameResponse = avcodec_send_frame(&encoderContext, &frame);
    if (sendFrameResponse < 0)
    {
        throw std::runtime_error("Error while sending a frame for encoding.");
    }

    AVPacket packet = {nullptr};
    av_init_packet(&packet);
    packet.flags |= AV_PKT_FLAG_KEY;

    if (avcodec_receive_packet(&encoderContext, &packet) == 0)
    {
        av_interleaved_write_frame(*_output.context, &packet);
        av_packet_unref(&packet);
    }
}

std::shared_ptr<pf::util::VideoEncoder> pf::util::VideoEncoder::cbr(std::filesystem::path path,
                                                                    size_t width,
                                                                    size_t height,
                                                                    size_t fps,
                                                                    uint64_t bitrate)
{
    // (I don't want a public constructor.)
    auto videoEncoder = std::make_shared<VideoEncoder>();
    videoEncoder->_filePath = std::move(path);
    videoEncoder->_width = width;
    videoEncoder->_height = height;
    videoEncoder->_fps = fps;
    videoEncoder->_bitrate = bitrate;

    return videoEncoder;
}

std::shared_ptr<pf::util::VideoEncoder> pf::util::VideoEncoder::crf(std::filesystem::path path,
                                                                    size_t width,
                                                                    size_t height,
                                                                    size_t fps,
                                                                    size_t crf)
{

    std::shared_ptr<VideoEncoder> videoEncoder = cbr(std::move(path), width, height, fps, 0);
    videoEncoder->_parametersSetter = [crf](AVCodecContext &encoderContext, AVStream & /*stream*/) {
        av_opt_set_int(
            &encoderContext, "crf", gsl::narrow_cast<int64_t>(crf), AV_OPT_SEARCH_CHILDREN);
    };

    return videoEncoder;
}

std::shared_ptr<pf::util::VideoEncoder>
pf::util::VideoEncoder::gif(std::filesystem::path path, size_t width, size_t height, size_t fps)
{
    path.replace_extension(".gif");
    std::shared_ptr<VideoEncoder> videoEncoder = cbr(std::move(path), width, height, fps, 0);
    videoEncoder->_parametersSetter = [](AVCodecContext & /*encoderContext*/, AVStream &stream) {
        stream.codecpar->format = AV_PIX_FMT_RGB8;
    };
    return videoEncoder;
}

std::filesystem::path
pf::util::VideoEncoder::fixOutputFilePath(std::filesystem::path const &originalPath)
{
    std::filesystem::path fixedPath = originalPath;
    if (fixedPath.empty())
    {
        fixedPath = std::filesystem::current_path();
    }
    if (!fixedPath.has_filename())
    {
        fixedPath.replace_filename(DEFAULT_OUTPUT_FILE_NAME);
    }
    if (!fixedPath.has_extension())
    {
        fixedPath.replace_extension(DEFAULT_OUTPUT_FILE_EXTENSION);
    }
    if (fixedPath.has_parent_path())
    {
        std::filesystem::create_directories(fixedPath.parent_path());
    }
    if (std::filesystem::exists(fixedPath))
    {
        fixedPath = pf::util::file::uniquePath(fixedPath).value_or(fixedPath);
    }

    return fixedPath;
}

AVOutputFormat *pf::util::VideoEncoder::Output::guessFormat(std::filesystem::path const &filePath)
{
    AVOutputFormat *format =
        av_guess_format(nullptr, filePath.filename().string().c_str(), nullptr);
    if (format == nullptr)
    {
        throw std::runtime_error(
            fmt::format("Failed to guess an output format based on file name ({}).",
                        filePath.filename().string()));
    }
    return format;
}

void pf::util::VideoEncoder::yFlippedImageCopy(RGB *destination,
                                               RGB const *source,
                                               size_t width,
                                               size_t height)
{
    RGB const *sourceIterator = source;
    RGB const *sourceEnd = source + width * height;
    RGB *destinationIterator = destination + width * height;

    while (sourceIterator < sourceEnd)
    {
        destinationIterator -= width;
        std::memcpy(destinationIterator, sourceIterator, width * sizeof(RGB));
        sourceIterator += width;
    }
}

void pf::util::VideoEncoder::linewiseImageCopy(uint8_t *destination,
                                               size_t destinationLineSize,
                                               uint8_t const *source,
                                               size_t sourceLineSize,
                                               size_t width,
                                               size_t height)
{
    uint8_t const *sourceIterator = source;
    uint8_t const *sourceEnd = source + sourceLineSize * height;
    uint8_t *destinationIterator = destination;
    while (sourceIterator < sourceEnd)
    {
        std::memcpy(destinationIterator, sourceIterator, width);
        sourceIterator += sourceLineSize;
        destinationIterator += destinationLineSize;
    }
}
