#ifndef VIDEO_ENCODER_HPP
#define VIDEO_ENCODER_HPP

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

/**
 * A wrapper class for ffmpeg functions used to write video to disk from a sequence of raw RGB data.
 */
class VideoEncoder
{
public:
    static std::shared_ptr<VideoEncoder>
    cbr(std::filesystem::path path, size_t width, size_t height, size_t fps, uint64_t bitrate);

    static std::shared_ptr<VideoEncoder>
    crf(std::filesystem::path path, size_t width, size_t height, size_t fps, size_t crf);

    static std::shared_ptr<VideoEncoder>
    gif(std::filesystem::path path, size_t width, size_t height, size_t fps);

    /**
     * Creates a dummy video encoder.
     */
    VideoEncoder();

    VideoEncoder(VideoEncoder const &) = delete;
    VideoEncoder(VideoEncoder &&) = default;

    virtual ~VideoEncoder();

    VideoEncoder &operator=(VideoEncoder const &) = delete;
    VideoEncoder &operator=(VideoEncoder &&) = default;

    [[nodiscard]] bool started() const;
    [[nodiscard]] bool finished() const;
    [[nodiscard]] size_t framesCount() const;

    void start();
    void appendFrameFromRGB(std::vector<uint8_t> const &rgbData);
    void finish();

private:
    template <typename T>
    using UniquePointer = std::unique_ptr<T, void (*)(T *)>;

    using EncodingParametersSetter =
        std::function<void(AVCodecContext &encoderContext, AVStream &outputStream)>;

    struct Output
    {
        UniquePointer<AVFormatContext *> context =
            UniquePointer<AVFormatContext *>(nullptr, nullptr);
        AVStream *stream = nullptr;
        AVOutputFormat *format = nullptr;

        static AVOutputFormat *guessFormat(std::filesystem::path const &filePath);
    };

    struct Encoder
    {
        UniquePointer<AVCodecContext> context = UniquePointer<AVCodecContext>(nullptr, nullptr);
        AVCodec *codec = nullptr;
    };

    struct Image
    {
        UniquePointer<uint8_t *> data = UniquePointer<uint8_t *>(nullptr, nullptr);
        std::array<int, 4> lineSize = {};
    };

    struct RGB
    {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    static std::string const DEFAULT_OUTPUT_FILE_NAME;
    static std::string const DEFAULT_CODEC_NAME;
    static std::string const DEFAULT_OUTPUT_FILE_EXTENSION;

    static std::filesystem::path fixOutputFilePath(std::filesystem::path const &originalPath);
    static void yFlippedImageCopy(RGB *destination, RGB const *source, size_t width, size_t height);

    static void linewiseImageCopy(uint8_t *destination,
                                  size_t destinationLineSize,
                                  uint8_t const *source,
                                  size_t sourceLineSize,
                                  size_t width,
                                  size_t height);

    VideoEncoder(std::filesystem::path path,
                 size_t width,
                 size_t height,
                 size_t fps,
                 uint64_t bitrate);

    void encodeFrame(AVCodecContext &encoderContext, AVFrame &frame);

    bool _started = false, _finished = false;
    size_t _frameIndex = 0;
    std::filesystem::path _filePath;
    size_t _width, _height;
    size_t _fps;
    uint64_t _bitrate;

    Output _output;
    Encoder _encoder;
    UniquePointer<SwsContext> _swsContext;
    Image _source, _destination;
    UniquePointer<AVFrame> _frame;

    /**
     * This function is called somewhere inside the `start` method, it can be used to inject
     * additional parameters of the encoder / output stream.
     */
    EncodingParametersSetter _parametersSetter = [](AVCodecContext & /*encoderContext*/,
                                                    AVStream & /*stream*/) {};
};

} // namespace pf::util

#endif // !VIDEO_ENCODER_HPP
