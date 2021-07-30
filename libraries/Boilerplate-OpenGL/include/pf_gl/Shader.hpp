#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>
#include <memory>
#include <filesystem>

#include <pf_gl/Window.hpp>
#include <pf_gl/ValueTypes.hpp>

namespace pf::gl
{

enum ShaderType
{
    VERTEX_SHADER,
    FRAGMENT_SHADER,
    COMPUTE_SHADER,
};

class Shader final
{
public:
    /**
     * A dummy shader program. Trying using it will result in `std::runtime_error` exception thrown.
     */
    Shader();

    Shader(Shader const &) = delete;
    Shader(Shader &&) = default;

    Shader(std::shared_ptr<Window> window,
           std::filesystem::path const &vertexShaderPath,
           std::filesystem::path const &fragmentShaderPath);

    Shader(std::shared_ptr<Window> window,
           std::filesystem::path const &shaderPath,
           ShaderType shaderType);

    ~Shader();

    Shader &operator=(Shader const &) = delete;
    Shader &operator=(Shader &&) = default;

    void use() const;
    void unbind() const;

    template <typename T>
    void setUniformValue(std::string const &name, T value);

private:
    types::UInt _id;
    std::shared_ptr<Window> _window;

    GLint getUniformLocation(std::string const &name);
};


// * Templates definitions *

template <typename T>
void Shader::setUniformValue(std::string const & /*name*/, T /*value*/)
{
    throw std::logic_error("This uniform type is not supported.");
}

template <>
void Shader::setUniformValue<types::Float>(std::string const &name, types::Float value);

template <>
void Shader::setUniformValue<types::FVec3>(std::string const &name, types::FVec3 value);

template <>
void Shader::setUniformValue<types::FVec2>(std::string const &name, types::FVec2 value);

template <>
void Shader::setUniformValue<types::FMat4>(std::string const &name, types::FMat4 value);

template <>
void Shader::setUniformValue<types::IntVec2>(std::string const &name, types::IntVec2 value);

template <>
void Shader::setUniformValue<types::Int>(std::string const &name, types::Int value);

template <>
void Shader::setUniformValue<types::Bool>(std::string const &name, types::Bool value);

} // namespace pf::gl

#endif // !SHADER_HPP
