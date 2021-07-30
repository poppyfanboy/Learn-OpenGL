/**
 * A wrapper for ffmpeg functions used to write video to disk from a sequence of raw RGB data.
 */

#ifndef VIDEO_ENCODER_H
#define VIDEO_ENCODER_H

extern "C"
{
#include <libavutil/frame.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <cstdint>
#include <string>
#include <memory>
#include <fstream>
#include <filesystem>
#include <vector>
#include <functional>
#include <array>

namespace pf::util
{

class VideoEncoder
{
public:
    static std::shared_ptr<VideoEncoder> getConstantBitrateVideoEncoder(std::filesystem::path path,
                                                                        size_t width,
                                                                        size_t height,
                                                                        size_t fps,
                                                                        uint64_t bitrate);

    static std::shared_ptr<VideoEncoder> getCrfVideoEncoder(std::filesystem::path path,
                                                            size_t width,
                                                            size_t height,
                                                            size_t fps,
                                                            size_t crf);
    /**
     * @brief The resulting GIF will have 256 colors.
     */
    static std::shared_ptr<VideoEncoder>
    getLowQualityGifEncoder(std::filesystem::path path, size_t width, size_t height, size_t fps);

    // A dummy video encoder, not intended to be used
    VideoEncoder();

    VideoEncoder(VideoEncoder const &) = delete;
    VideoEncoder(VideoEncoder &&) = delete;

    virtual ~VideoEncoder();

    void startEncoding();
    void addFrameFromRGB(std::vector<uint8_t> const &rgbData);
    void finishEncoding();
    bool encodingStarted() const;
    bool encodingEnded() const;
    size_t getFramesCount() const;

    VideoEncoder &operator=(VideoEncoder &&) = delete;
    VideoEncoder &operator=(VideoEncoder const &) = delete;

private:
    using EncodingParametersSetter =
        std::function<void(std::shared_ptr<AVCodecContext> encodingContext,
                           AVStream *outputStream)>;

    static std::string const DEFAULT_CODEC_NAME;
    static std::string const DEFAULT_OUTPUT_FILE_EXTENSION;

    VideoEncoder(std::filesystem::path path,
                 size_t width,
                 size_t height,
                 size_t fps,
                 uint64_t bitrate);

    void _encodeFrame(std::shared_ptr<AVCodecContext> encoderContext,
                      std::shared_ptr<AVFrame> frame);

    bool _encodingStarted = false;
    bool _encodingEnded = false;
    size_t _frameIndex = 0;
    std::filesystem::path _filePath;
    size_t _width, _height;
    size_t _fps;
    uint64_t _bitrate;

    // two buffers used to convert pixels from RGB into YCbCr color space
    std::unique_ptr<uint8_t *, std::function<void(uint8_t **)>> _sourceData;
    std::array<int, 4> _sourceLineSize = {};
    std::unique_ptr<uint8_t *, std::function<void(uint8_t **)>> _destinationData;
    std::array<int, 4> _destinationLineSize = {};

    std::shared_ptr<SwsContext> _swsContext;
    std::shared_ptr<AVFrame> _frame;
    std::shared_ptr<AVCodecContext> _encoderContext;
    std::shared_ptr<AVFormatContext *> _outputContext;
    // AVCodecContext manages streams that belong to it, no need to delete them manually
    AVStream *_outputStream = nullptr;

    AVCodec *_codec = nullptr;
    AVOutputFormat *_outputFormat = nullptr;

    EncodingParametersSetter _parametersSetter = [](std::shared_ptr<AVCodecContext>, AVStream *) {};
};

} // namespace pf::util

#endif // !VIDEO_ENCODER_H
