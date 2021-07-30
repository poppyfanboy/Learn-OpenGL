#include <cstdio>
#include <pf_utils/VideoEncoder.hpp>

#include <filesystem>
#include <functional>
#include <iostream>
#include <libavutil/pixfmt.h>
#include <stdexcept>
#include <string>
#include <utility>
#include <memory>
#include <array>
#include <fstream>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavformat/avformat.h>
#include <libavutil/time.h>
}

#include <pf_utils/FileUtils.hpp>

std::string const pf::util::VideoEncoder::DEFAULT_CODEC_NAME = "libx264";
std::string const pf::util::VideoEncoder::DEFAULT_OUTPUT_FILE_EXTENSION = ".mp4";

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
{
}

pf::util::VideoEncoder::VideoEncoder()
    : _filePath(std::filesystem::path(""))
    , _width(1)
    , _height(1)
    , _fps(1)
    , _bitrate(1000)
{
}

pf::util::VideoEncoder::~VideoEncoder()
{
    if (!_encodingEnded)
    {
        finishEncoding();
    }
}

void pf::util::VideoEncoder::startEncoding()
{
    av_log_set_level(AV_LOG_QUIET);

    if (_filePath.empty() || !_filePath.has_filename())
    {
        throw std::runtime_error("Invalid path: " + _filePath.string());
    }
    if (_filePath.has_parent_path())
    {
        std::filesystem::create_directories(_filePath.parent_path());
    }

    if (std::filesystem::exists(_filePath))
    {
        _filePath = pf::util::file::uniquePath(_filePath).value_or(_filePath);
    }

    _outputFormat = av_guess_format(nullptr, _filePath.filename().string().c_str(), nullptr);
    if (_outputFormat == nullptr)
    {
        throw std::runtime_error("Failed to guess an output format based of file name (" +
                                 _filePath.filename().string() + ").");
    }

    _outputContext = std::shared_ptr<AVFormatContext *>(new AVFormatContext *,
                                                        [](AVFormatContext **outputContext)
                                                        {
                                                            avformat_free_context(*outputContext);
                                                            delete outputContext;
                                                        });
    int outputContextAllocResponse = avformat_alloc_output_context2(
        _outputContext.get(), _outputFormat, nullptr, _filePath.string().c_str());
    if (outputContextAllocResponse != 0)
    {
        throw std::runtime_error("Failed to allocate output context.");
    }

    _codec = avcodec_find_encoder(_outputFormat->video_codec);
    if (_codec == nullptr)
    {
        // Use the default codec instead
        _filePath.replace_extension(DEFAULT_OUTPUT_FILE_EXTENSION);
        _codec = avcodec_find_encoder_by_name(DEFAULT_CODEC_NAME.c_str());
        if (_codec == nullptr)
        {
            throw std::runtime_error("Failed to find both fallback and user specified codecs.");
        }
    }

    std::ofstream fileOutput(_filePath, std::ios::binary);
    if (fileOutput.fail())
    {
        throw std::runtime_error("Failed to open file " + _filePath.string());
    }
    fileOutput.close();

    _outputStream = avformat_new_stream(*_outputContext, _codec);
    if (_outputStream == nullptr)
    {
        throw std::runtime_error("Failed to create an output stream.");
    }

    _encoderContext = std::shared_ptr<AVCodecContext>(avcodec_alloc_context3(_codec),
                                                      [](AVCodecContext *encoderContext)
                                                      { avcodec_free_context(&encoderContext); });
    if (_encoderContext == nullptr)
    {
        throw std::runtime_error("Failed to allocated encoder context");
    }

    _outputStream->codecpar->codec_id = _outputFormat->video_codec;
    _outputStream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    _outputStream->codecpar->width = _width;
    _outputStream->codecpar->height = _height;
    _outputStream->codecpar->format = AV_PIX_FMT_YUV420P;
    _outputStream->codecpar->bit_rate = _bitrate;
    _parametersSetter(_encoderContext, _outputStream);
    avcodec_parameters_to_context(_encoderContext.get(), _outputStream->codecpar);

    _encoderContext->time_base = {1, static_cast<int>(_fps)};
    _encoderContext->framerate = {static_cast<int>(_fps), 1};
    _encoderContext->gop_size = 10;
    _encoderContext->max_b_frames = 1;

    if (_outputStream->codecpar->codec_id == AV_CODEC_ID_H264)
    {
        av_opt_set(_encoderContext->priv_data, "preset", "slow", 0);
    }
    _parametersSetter(_encoderContext, _outputStream);
    avcodec_parameters_from_context(_outputStream->codecpar, _encoderContext.get());

    int openCodecResponse = avcodec_open2(_encoderContext.get(), _codec, nullptr);
    if (openCodecResponse < 0)
    {
        std::array<char, 64> responseString = {0};
        throw std::runtime_error("Failed to open codec " +
                                 std::string(av_make_error_string(responseString.data(),
                                                                  responseString.size(),
                                                                  openCodecResponse)));
    }

    if ((_outputFormat->flags & AVFMT_NOFILE) == 0)
    {
        int fileOpenResponse =
            avio_open(&(*_outputContext)->pb, _filePath.string().c_str(), AVIO_FLAG_WRITE);
        if (fileOpenResponse < 0)
        {
            throw std::runtime_error("Failed to open file " + _filePath.string());
        }
    }

    int writeHeaderResponse = avformat_write_header(*_outputContext, nullptr);
    if (writeHeaderResponse < 0)
    {
        throw std::runtime_error("Failed to write file header.");
    }
    av_dump_format(*_outputContext, 0, _filePath.string().c_str(), 1);

    _frame =
        std::shared_ptr<AVFrame>(av_frame_alloc(), [](AVFrame *frame) { av_frame_free(&frame); });
    if (_frame == nullptr)
    {
        throw std::runtime_error("Failed to allocate a video frame.");
    }
    _frame->format = _encoderContext->pix_fmt;
    _frame->width = _encoderContext->width;
    _frame->height = _encoderContext->height;
    _frame->pts = 0;

    int getBufferResponse = av_frame_get_buffer(_frame.get(), 0);
    if (getBufferResponse < 0)
    {
        throw std::runtime_error("Failed to allocate a buffer");
    }

    _sourceData = std::unique_ptr<uint8_t *, std::function<void(uint8_t **)>>(new uint8_t *[4],
                                                                              [](uint8_t **data)
                                                                              {
                                                                                  av_freep(data);
                                                                                  delete[] data;
                                                                              });
    AVPixelFormat sourcePixelFormat = AV_PIX_FMT_RGB24;
    int sourceImageAllocResponse = av_image_alloc(
        _sourceData.get(), _sourceLineSize.data(), _width, _height, sourcePixelFormat, 1);
    if (sourceImageAllocResponse < 0)
    {
        throw std::runtime_error("Could not allocate source image");
    }

    _destinationData =
        std::unique_ptr<uint8_t *, std::function<void(uint8_t **)>>(new uint8_t *[4],
                                                                    [](uint8_t **data)
                                                                    {
                                                                        av_freep(data);
                                                                        delete[] data;
                                                                    });
    AVPixelFormat destinationPixelFormat =
        static_cast<AVPixelFormat>(_outputStream->codecpar->format);
    int destinationImageAllocResponse = av_image_alloc(_destinationData.get(),
                                                       _destinationLineSize.data(),
                                                       _width,
                                                       _height,
                                                       destinationPixelFormat,
                                                       4);
    if (destinationImageAllocResponse < 0)
    {
        throw std::runtime_error("Could not allocate destination image");
    }

    _swsContext =
        std::shared_ptr<SwsContext>(sws_getContext(_width,
                                                   _height,
                                                   sourcePixelFormat,
                                                   _width,
                                                   _height,
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

    _encodingStarted = true;
}

void pf::util::VideoEncoder::addFrameFromRGB(std::vector<uint8_t> const &rgbData)
{
    if (_encodingEnded)
    {
        throw std::runtime_error("Cannot add a new frame after encoding has ended.");
    }
    if (!_encodingStarted)
    {
        startEncoding();
    }
    if (rgbData.size() != 3 * _width * _height)
    {
        throw std::runtime_error(
            "Amount of the passed RGB bytes does not match with the dimensions of the video");
    }

    size_t pixelsCount = _width * _height;
    for (size_t y = 0; y < _height; y++)
    {
        for (size_t x = 0; x < _width; x++)
        {
            size_t pixelIndex = y * _width + x;
            _sourceData.get()[0][y * _sourceLineSize[0] + 3 * x] =
                rgbData.at(3 * (pixelsCount - pixelIndex - 1));
            _sourceData.get()[0][y * _sourceLineSize[0] + 3 * x + 1] =
                rgbData.at(3 * (pixelsCount - pixelIndex - 1) + 1);
            _sourceData.get()[0][y * _sourceLineSize[0] + 3 * x + 2] =
                rgbData.at(3 * (pixelsCount - pixelIndex - 1) + 2);
        }
    }

    int makeWritableResponse = av_frame_make_writable(_frame.get());
    if (makeWritableResponse < 0)
    {
        throw std::runtime_error("Frame is not writable.");
    }


    sws_scale(_swsContext.get(),
              _sourceData.get(),
              _sourceLineSize.data(),
              0,
              _height,
              _destinationData.get(),
              _destinationLineSize.data());
    for (size_t y = 0; y < _height; y++)
    {
        for (size_t x = 0; x < _width; x++)
        {
            _frame->data[0][y * _frame->linesize[0] + x] =
                _destinationData.get()[0][y * _destinationLineSize[0] + x];
        }
    }
    if (_outputStream->codecpar->format == AV_PIX_FMT_YUV420P)
    {
        for (size_t y = 0; y < _height / 2; y++)
        {
            for (size_t x = 0; x < _width / 2; x++)
            {
                _frame->data[1][y * _frame->linesize[1] + x] =
                    _destinationData.get()[1][y * _destinationLineSize[1] + x];
                _frame->data[2][y * _frame->linesize[2] + x] =
                    _destinationData.get()[2][y * _destinationLineSize[2] + x];
            }
        }
    }

    _frame->pts =
        _frameIndex * _outputStream->time_base.den / (_outputStream->time_base.num * _fps);
    _encodeFrame(_encoderContext, _frame);
    _frameIndex++;
}

void pf::util::VideoEncoder::finishEncoding()
{
    if (_encodingEnded || !_encodingStarted)
    {
        throw std::runtime_error(
            "Either the encoding has already ended or it has not started yet.");
    }
    _encodingEnded = true;

    AVPacket packet = {nullptr};
    av_init_packet(&packet);

    // encode delayed frames
    while (true)
    {
        avcodec_send_frame(_encoderContext.get(), nullptr);
        if (avcodec_receive_packet(_encoderContext.get(), &packet) == 0)
        {
            av_interleaved_write_frame(*_outputContext, &packet);
            av_packet_unref(&packet);
        }
        else
        {
            break;
        }
    }

    av_write_trailer(*_outputContext);
    if ((_outputFormat->flags & AVFMT_NOFILE) == 0)
    {
        int fileCloseResponse = avio_close((*_outputContext)->pb);
        if (fileCloseResponse != 0)
        {
            throw std::runtime_error("Failed to close file " + _filePath.string());
        }
    }
}

bool pf::util::VideoEncoder::encodingStarted() const
{
    return _encodingStarted;
}

bool pf::util::VideoEncoder::encodingEnded() const
{
    return _encodingEnded;
}

size_t pf::util::VideoEncoder::getFramesCount() const
{
    return _frameIndex;
}

void pf::util::VideoEncoder::_encodeFrame(std::shared_ptr<AVCodecContext> encoderContext,
                                          std::shared_ptr<AVFrame> frame)
{
    int sendFrameResponse = avcodec_send_frame(encoderContext.get(), frame.get());
    if (sendFrameResponse < 0)
    {
        throw std::runtime_error("Error while sending a frame for encoding.");
    }

    AVPacket packet = {nullptr};
    av_init_packet(&packet);
    packet.flags |= AV_PKT_FLAG_KEY;

    if (avcodec_receive_packet(encoderContext.get(), &packet) == 0)
    {
        av_interleaved_write_frame(*_outputContext, &packet);
        av_packet_unref(&packet);
    }
}

std::shared_ptr<pf::util::VideoEncoder>
pf::util::VideoEncoder::getConstantBitrateVideoEncoder(std::filesystem::path path,
                                                       size_t width,
                                                       size_t height,
                                                       size_t fps,
                                                       uint64_t bitrate)
{
    // (I don't want a public constructor.)
    auto videoEncoder = std::make_shared<VideoEncoder>();
    videoEncoder->_filePath = path;
    videoEncoder->_width = width;
    videoEncoder->_height = height;
    videoEncoder->_fps = fps;
    videoEncoder->_bitrate = bitrate;

    return videoEncoder;
}

std::shared_ptr<pf::util::VideoEncoder>
pf::util::VideoEncoder::getCrfVideoEncoder(std::filesystem::path path,
                                           size_t width,
                                           size_t height,
                                           size_t fps,
                                           size_t crf)
{

    std::shared_ptr<VideoEncoder> videoEncoder =
        getConstantBitrateVideoEncoder(std::move(path), width, height, fps, 0);
    videoEncoder->_parametersSetter =
        [crf](const std::shared_ptr<AVCodecContext> &encoderContext, AVStream * /*stream*/)
    { av_opt_set_int(encoderContext.get(), "crf", crf, AV_OPT_SEARCH_CHILDREN); };

    return videoEncoder;
}

std::shared_ptr<pf::util::VideoEncoder>
pf::util::VideoEncoder::getLowQualityGifEncoder(std::filesystem::path path,
                                                size_t width,
                                                size_t height,
                                                size_t fps)
{
    path.replace_extension(".gif");
    std::shared_ptr<VideoEncoder> videoEncoder =
        getConstantBitrateVideoEncoder(std::move(path), width, height, fps, 0);
    videoEncoder->_parametersSetter =
        [](std::shared_ptr<AVCodecContext> /*encoderContext*/, AVStream *stream)
    { stream->codecpar->format = AV_PIX_FMT_RGB8; };
    return videoEncoder;
}
