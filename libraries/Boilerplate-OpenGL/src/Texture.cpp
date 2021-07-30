#include <pf_gl/Texture.hpp>

#include <stdexcept>
#include <memory>
#include <utility>
#include <string>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <pf_gl/Window.hpp>
#include <pf_gl/ValueTypes.hpp>

namespace pf::gl
{

Texture::Texture(std::shared_ptr<Window> window,
                 std::filesystem::path const &filePath,
                 TextureType textureType,
                 types::Int wrapS,
                 types::Int wrapT,
                 types::Int minFilter,
                 types::Int magFilter)
    : _window(std::move(window))
    , _textureType(textureType)
    , _filePath(filePath)
{
    int channelsCount = 0;
    unsigned char *data =
        stbi_load(filePath.string().c_str(), &_width, &_height, &channelsCount, 0);
    if (data == nullptr)
    {
        throw std::runtime_error("Could not load the image " + filePath.string() + ".");
    }

    types::Int format = 0;
    if (channelsCount == 1)
    {
        format = GL_RED;
    }
    else if (channelsCount == 3)
    {
        format = GL_RGB;
    }
    else if (channelsCount == 4)
    {
        format = GL_RGBA;
    }

    _window->bindContext();

    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D, _texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

    glTexImage2D(GL_TEXTURE_2D, 0, format, _width, _height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture()
{
    _window->bindContext();
    glDeleteTextures(1, &_texture);
}

void Texture::bind() const
{
    _window->bindContext();
    glBindTexture(GL_TEXTURE_2D, _texture);
}

TextureType Texture::type() const
{
    return _textureType;
}

std::filesystem::path Texture::filePath() const
{
    return _filePath;
}

void Texture::unbind() const
{
    _window->bindContext();
    glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace pf::gl
