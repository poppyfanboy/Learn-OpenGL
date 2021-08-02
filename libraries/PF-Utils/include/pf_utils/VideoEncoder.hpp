#ifndef VIDEO_ENCODER_HPP
#define VIDEO_ENCODER_HPP

extern "C"
{
#include <libavutil/frame.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixfmt.h>
}

#include <cstdint>
#include <string>
#include <memory>
#include <fstream>
#include <filesystem>
#include <vector>
#include <functional>
#include <array>
#include <span>

namespace pf::util
{

/**
 * A wrapper class for ffmpeg functions used to write video to disk from a sequence of raw RGB data.
 */
class VideoEncoder final
{
public:
    static std::unique_ptr<VideoEncoder>
    cbr(std::filesystem::path path, size_t width, size_t height, size_t fps, uint64_t bitrate);

    static std::unique_ptr<VideoEncoder>
    crf(std::filesystem::path path, size_t width, size_t height, size_t fps, size_t crf);

    static std::unique_ptr<VideoEncoder>
    gif(std::filesystem::path path, size_t width, size_t height, size_t fps);

    /**
     * Creates a dummy video encoder.
     */
    VideoEncoder();

    VideoEncoder(VideoEncoder const &) = delete;
    VideoEncoder(VideoEncoder &&) = default;

    ~VideoEncoder();

    VideoEncoder &operator=(VideoEncoder const &) = delete;
    VideoEncoder &operator=(VideoEncoder &&) = default;

    [[nodiscard]] bool started() const;
    [[nodiscard]] bool finished() const;
    [[nodiscard]] size_t framesCount() const;

    void start();
    void appendFrameFromRGB(std::span<uint8_t> const &sourceRGB);
    void finish();

private:
    template <typename T>
    using UniquePointer = std::unique_ptr<T, void (*)(T *)>;

    using EncodingParametersSetter =
        std::function<void(AVCodecContext &encoderContext, AVStream &outputStream)>;

    struct Encoder;

    struct Output
    {
        UniquePointer<AVFormatContext *> context =
            UniquePointer<AVFormatContext *>(nullptr, nullptr);
        AVStream *stream = nullptr;
        AVOutputFormat *format = nullptr;

        std::filesystem::path filePath;
        size_t width, height;
        size_t fps;
        uint64_t bitrate;

        static AVOutputFormat *guessFormat(std::filesystem::path const &filePath);

        Output(std::filesystem::path filePath,
               size_t width,
               size_t height,
               size_t fps,
               uint64_t bitrate);

        void createContext();
        void createStream(Encoder &encoder, EncodingParametersSetter &parametersSetter);
        void writeHeader();
        void close();
    };

    struct Encoder
    {
        UniquePointer<AVCodecContext> context = UniquePointer<AVCodecContext>(nullptr, nullptr);
        AVCodec *codec = nullptr;
        UniquePointer<AVFrame> frame = UniquePointer<AVFrame>(nullptr, nullptr);

        Encoder() = default;
        explicit Encoder(Output &output);

        void openCodec();
        void createFrame();
    };

    struct Image
    {
        UniquePointer<uint8_t *> data = UniquePointer<uint8_t *>(nullptr, nullptr);
        std::array<int, 4> lineSize = {};
        size_t width, height;
        AVPixelFormat pixelFormat;

        Image();
        Image(size_t width, size_t height, AVPixelFormat pixelFormat, size_t align);
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

    Output _output;
    Encoder _encoder;
    UniquePointer<SwsContext> _swsContext;
    Image _rgbImage, _convertedImage;

    /**
     * This function is called somewhere inside the `start` method, it can be used to inject
     * additional parameters of the encoder / output stream.
     */
    EncodingParametersSetter _parametersSetter = [](AVCodecContext & /*encoderContext*/,
                                                    AVStream & /*stream*/) {};
};

} // namespace pf::util

#endif // !VIDEO_ENCODER_HPP
