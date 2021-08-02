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
#include <cassert>

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

namespace pf::util
{

std::string const VideoEncoder::DEFAULT_CODEC_NAME = "libx264";
std::string const VideoEncoder::DEFAULT_OUTPUT_FILE_EXTENSION = ".mp4";
std::string const VideoEncoder::DEFAULT_OUTPUT_FILE_NAME = "Video";

VideoEncoder::VideoEncoder(std::filesystem::path filePath,
                           size_t width,
                           size_t height,
                           size_t fps,
                           uint64_t bitrate)
    : _output(std::move(filePath), width, height, fps, bitrate)
    , _swsContext(nullptr, nullptr)
{
}

VideoEncoder::VideoEncoder()
    : VideoEncoder(std::filesystem::path(), 1, 1, 1, 1000)
{
}

VideoEncoder::~VideoEncoder()
{
    if (started() && !finished())
    {
        finish();
    }
}

void VideoEncoder::start()
{
    // TODO(poppyfanboy) Add a way to connect a logger to the encoder and redirect AV lib logs
    av_log_set_level(AV_LOG_QUIET);

    _output.filePath = fixOutputFilePath(_output.filePath);
    _output.format = Output::guessFormat(_output.filePath);

    _encoder = Encoder(_output);

    _output.createContext();
    _output.createStream(_encoder, _parametersSetter);

    _encoder.openCodec();
    _output.writeHeader();
    _encoder.createFrame();

    // * Create images to convert between color spaces and create SWS context *

    _rgbImage = Image(_output.width, _output.height, AV_PIX_FMT_RGB24, 1);

    _convertedImage = Image(_output.width,
                            _output.height,
                            static_cast<AVPixelFormat>(_output.stream->codecpar->format),
                            4);

    _swsContext =
        UniquePointer<SwsContext>(sws_getContext(gsl::narrow_cast<int>(_rgbImage.width),
                                                 gsl::narrow_cast<int>(_rgbImage.height),
                                                 _rgbImage.pixelFormat,
                                                 gsl::narrow_cast<int>(_convertedImage.width),
                                                 gsl::narrow_cast<int>(_convertedImage.height),
                                                 _convertedImage.pixelFormat,
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

void VideoEncoder::appendFrameFromRGB(std::span<uint8_t> const &sourceRGB)
{
    if (_finished)
    {
        throw std::runtime_error("Cannot add a new frame after encoding has ended.");
    }
    if (!_started)
    {
        start();
    }
    if (sourceRGB.size() != 3 * _output.width * _output.height)
    {
        throw std::runtime_error(
            "Amount of the passed RGB bytes does not match with the dimensions of the video");
    }

    // Flip vertically before converting to YCbCr
    yFlippedImageCopy(reinterpret_cast<RGB *>(_rgbImage.data.get()[0]),
                      reinterpret_cast<RGB const *>(sourceRGB.data()),
                      _rgbImage.width,
                      _rgbImage.height);

    int makeWritableResponse = av_frame_make_writable(_encoder.frame.get());
    if (makeWritableResponse < 0)
    {
        throw std::runtime_error("Frame is not writable.");
    }
    sws_scale(_swsContext.get(),
              _rgbImage.data.get(),
              _rgbImage.lineSize.data(),
              0,
              gsl::narrow_cast<int>(_rgbImage.height),
              _convertedImage.data.get(),
              _convertedImage.lineSize.data());

    linewiseImageCopy(_encoder.frame->data[0],
                      _encoder.frame->linesize[0],
                      _convertedImage.data.get()[0],
                      _convertedImage.lineSize[0],
                      _convertedImage.width,
                      _convertedImage.height);

    if (_output.stream->codecpar->format == AV_PIX_FMT_YUV420P)
    {
        linewiseImageCopy(_encoder.frame->data[1],
                          _encoder.frame->linesize[1],
                          _convertedImage.data.get()[1],
                          _convertedImage.lineSize[1],
                          _convertedImage.width / 2,
                          _convertedImage.height / 2);

        linewiseImageCopy(_encoder.frame->data[2],
                          _encoder.frame->linesize[2],
                          _convertedImage.data.get()[2],
                          _convertedImage.lineSize[2],
                          _convertedImage.width / 2,
                          _convertedImage.height / 2);
    }

    _encoder.frame->pts = gsl::narrow_cast<int64_t>(_frameIndex) * _output.stream->time_base.den /
                          (_output.stream->time_base.num * gsl::narrow_cast<int64_t>(_output.fps));
    encodeFrame(*_encoder.context, *_encoder.frame);
    _frameIndex++;
}

void VideoEncoder::finish()
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

    _output.close();
}

bool VideoEncoder::started() const
{
    return _started;
}

bool VideoEncoder::finished() const
{
    return _finished;
}

size_t VideoEncoder::framesCount() const
{
    return _frameIndex;
}

void VideoEncoder::encodeFrame(AVCodecContext &encoderContext, AVFrame &frame)
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

std::unique_ptr<VideoEncoder> VideoEncoder::cbr(std::filesystem::path path,
                                                size_t width,
                                                size_t height,
                                                size_t fps,
                                                uint64_t bitrate)
{
    // (I don't want a public constructor.)
    auto videoEncoder = std::make_unique<VideoEncoder>();
    videoEncoder->_output = Output(std::move(path), width, height, fps, bitrate);

    return videoEncoder;
}

std::unique_ptr<VideoEncoder>
VideoEncoder::crf(std::filesystem::path path, size_t width, size_t height, size_t fps, size_t crf)
{

    auto videoEncoder = cbr(std::move(path), width, height, fps, 0);
    videoEncoder->_parametersSetter = [crf](AVCodecContext &encoderContext, AVStream & /*stream*/) {
        av_opt_set_int(
            &encoderContext, "crf", gsl::narrow_cast<int64_t>(crf), AV_OPT_SEARCH_CHILDREN);
    };

    return videoEncoder;
}

std::unique_ptr<VideoEncoder>
VideoEncoder::gif(std::filesystem::path path, size_t width, size_t height, size_t fps)
{
    path.replace_extension(".gif");
    auto videoEncoder = cbr(std::move(path), width, height, fps, 0);
    videoEncoder->_parametersSetter = [](AVCodecContext & /*encoderContext*/, AVStream &stream) {
        stream.codecpar->format = AV_PIX_FMT_RGB8;
    };
    return videoEncoder;
}

std::filesystem::path VideoEncoder::fixOutputFilePath(std::filesystem::path const &originalPath)
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
        fixedPath = file::uniquePath(fixedPath).value_or(fixedPath);
    }

    return fixedPath;
}


// * Output *

AVOutputFormat *VideoEncoder::Output::guessFormat(std::filesystem::path const &filePath)
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

VideoEncoder::Output::Output(std::filesystem::path filePath,
                             size_t width,
                             size_t height,
                             size_t fps,
                             uint64_t bitrate)
    : filePath(std::move(filePath))
    , width(width)
    , height(height)
    , fps(std::max(fps, static_cast<size_t>(1)))
    , bitrate(bitrate)
{
}

void VideoEncoder::Output::createContext()
{
    assert(format != nullptr);

    context = UniquePointer<AVFormatContext *>(new AVFormatContext *,
                                               [](AVFormatContext **outputContext) {
                                                   avformat_free_context(*outputContext);
                                                   delete outputContext;
                                               });
    int outputContextAllocResponse =
        avformat_alloc_output_context2(context.get(), format, nullptr, filePath.string().c_str());
    if (outputContextAllocResponse != 0)
    {
        throw std::runtime_error("Failed to allocate output context.");
    }
}

void VideoEncoder::Output::createStream(Encoder &encoder,
                                        EncodingParametersSetter &parametersSetter)
{
    assert(format != nullptr && context != nullptr);
    assert(encoder.codec != nullptr && encoder.context != nullptr);

    stream = avformat_new_stream(*context, encoder.codec);
    if (stream == nullptr)
    {
        throw std::runtime_error("Failed to create an output stream.");
    }

    stream->codecpar->codec_id = format->video_codec;
    stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    stream->codecpar->width = gsl::narrow_cast<int>(width);
    stream->codecpar->height = gsl::narrow_cast<int>(height);
    stream->codecpar->format = AV_PIX_FMT_YUV420P;
    stream->codecpar->bit_rate = gsl::narrow_cast<int64_t>(bitrate);
    if (stream->codecpar->codec_id == AV_CODEC_ID_H264 ||
        stream->codecpar->codec_id == AV_CODEC_ID_H265)
    {
        av_opt_set(encoder.context->priv_data, "preset", "slow", 0);
    }

    parametersSetter(*encoder.context, *stream);
    avcodec_parameters_to_context(encoder.context.get(), stream->codecpar);

    encoder.context->time_base = {1, static_cast<int>(fps)};
    encoder.context->framerate = {static_cast<int>(fps), 1};
    encoder.context->gop_size = 10;
    encoder.context->max_b_frames = 1;

    parametersSetter(*encoder.context, *stream);
    avcodec_parameters_from_context(stream->codecpar, encoder.context.get());
}

void VideoEncoder::Output::writeHeader()
{
    assert(format != nullptr);
    assert(context != nullptr);
    assert(stream != nullptr);

    // * Open file *

    if ((format->flags & AVFMT_NOFILE) == 0)
    {
        int fileOpenResponse =
            avio_open(&(*context)->pb, filePath.string().c_str(), AVIO_FLAG_WRITE);
        if (fileOpenResponse < 0)
        {
            throw std::runtime_error("Failed to open file " + filePath.string());
        }
    }

    // * Write header *

    int writeHeaderResponse = avformat_write_header(*context, nullptr);
    if (writeHeaderResponse < 0)
    {
        throw std::runtime_error("Failed to write file header.");
    }
    av_dump_format(*context, 0, filePath.string().c_str(), 1);
}

void VideoEncoder::Output::close()
{
    av_write_trailer(*context);
    if ((format->flags & AVFMT_NOFILE) == 0)
    {
        int fileCloseResponse = avio_close((*context)->pb);
        if (fileCloseResponse != 0)
        {
            throw std::runtime_error("Failed to close file " + filePath.string());
        }
    }
}


// * Encoder *

VideoEncoder::Encoder::Encoder(Output &output)
{
    assert(output.format != nullptr);

    codec = avcodec_find_encoder(output.format->video_codec);
    if (codec == nullptr)
    {
        // Use the default codec instead
        output.filePath.replace_extension(DEFAULT_OUTPUT_FILE_EXTENSION);
        output.format = Output::guessFormat(output.filePath);

        codec = avcodec_find_encoder(output.format->video_codec);
        if (codec == nullptr)
        {
            throw std::runtime_error("Failed to find both fallback and user specified codecs.");
        }
    }

    context = UniquePointer<AVCodecContext>(
        avcodec_alloc_context3(codec),
        [](AVCodecContext *encoderContext) { avcodec_free_context(&encoderContext); });
    if (context == nullptr)
    {
        throw std::runtime_error("Failed to allocate encoder context");
    }
}

void VideoEncoder::Encoder::openCodec()
{
    assert(context != nullptr);
    assert(codec != nullptr);

    int openCodecResponse = avcodec_open2(context.get(), codec, nullptr);
    if (openCodecResponse < 0)
    {
        std::array<char, 64> responseString = {0};
        throw std::runtime_error("Failed to open codec " +
                                 std::string(av_make_error_string(responseString.data(),
                                                                  responseString.size(),
                                                                  openCodecResponse)));
    }
}

void VideoEncoder::Encoder::createFrame()
{
    assert(codec != nullptr);
    assert(context != nullptr);

    frame = UniquePointer<AVFrame>(av_frame_alloc(), [](AVFrame *frame) { av_frame_free(&frame); });
    if (frame == nullptr)
    {
        throw std::runtime_error("Failed to allocate a video frame.");
    }
    frame->format = context->pix_fmt;
    frame->width = context->width;
    frame->height = context->height;
    frame->pts = 0;

    int getBufferResponse = av_frame_get_buffer(frame.get(), 0);
    if (getBufferResponse < 0)
    {
        throw std::runtime_error("Failed to allocate a buffer");
    }
}


// * Image *

VideoEncoder::Image::Image()
    : width(1)
    , height(1)
    , pixelFormat(AV_PIX_FMT_RGB24)
{
}

VideoEncoder::Image::Image(size_t width, size_t height, AVPixelFormat pixelFormat, size_t align)
    : width(width)
    , height(height)
    , pixelFormat(pixelFormat)
{
    data = UniquePointer<uint8_t *>(new uint8_t *[4], [](uint8_t **data) {
        av_freep(&data[0]);
        delete[] data;
    });
    int imageAllocResponse = av_image_alloc(data.get(),
                                            lineSize.data(),
                                            gsl::narrow_cast<int>(width),
                                            gsl::narrow_cast<int>(height),
                                            pixelFormat,
                                            align);
    if (imageAllocResponse < 0)
    {
        throw std::runtime_error("Could not allocate an image");
    }
}

void VideoEncoder::yFlippedImageCopy(RGB *destination,
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

void VideoEncoder::linewiseImageCopy(uint8_t *destination,
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

} // namespace pf::util
