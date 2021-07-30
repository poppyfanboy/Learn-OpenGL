#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <memory>
#include <filesystem>

#include <glad/glad.h>

#include <pf_gl/Window.hpp>
#include <pf_gl/ValueTypes.hpp>

namespace pf::gl
{

enum TextureType
{
    DIFFUSE,
    SPECULAR,
};

// TODO(poppyfanboy) Add an Image class which will be used to convert images to raw RGB(A) data.
// Pass `Image` class instances to `Texture` constructors.

// TODO(poppyfanboy) Replace GLenums with custom enums.

// TODO(poppyfanboy) Add a builder for the texture objects.

class Texture final
{
public:
    Texture(std::shared_ptr<Window> window,
            std::filesystem::path const &,
            TextureType textureType,
            types::Int wrapS = GL_CLAMP_TO_EDGE,
            types::Int wrapT = GL_CLAMP_TO_EDGE,
            types::Int minFilter = GL_LINEAR_MIPMAP_LINEAR,
            types::Int magFilter = GL_LINEAR);

    Texture(Texture const &) = delete;
    Texture(Texture &&) = default;

    ~Texture();

    Texture &operator=(Texture const &) = delete;
    Texture &operator=(Texture &&) = default;

    void bind() const;
    void unbind() const;
    [[nodiscard]] TextureType type() const;
    [[nodiscard]] std::filesystem::path filePath() const;

private:
    std::shared_ptr<Window> _window;
    TextureType _textureType;
    types::Size _width, _height;
    types::UInt _texture;
    std::filesystem::path _filePath;
};

} // namespace pf::gl

#endif // !TEXTURE_HPP
