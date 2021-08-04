#include <pf_gl/Shader.hpp>

#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <stdexcept>
#include <filesystem>

#include <glad/glad.h>
#include <fmt/format.h>
#include <sparsepp/spp.h>

#include <pf_gl/Window.hpp>
#include <pf_gl/ValueTypes.hpp>
#include <pf_utils/FileUtils.hpp>

namespace pf::gl
{

spp::sparse_hash_map<ShaderType, GLenum> const FROM_SHADER_TYPE_TO_GL_ENUM{
    {VERTEX_SHADER, GL_VERTEX_SHADER},
    {FRAGMENT_SHADER, GL_FRAGMENT_SHADER},
    {COMPUTE_SHADER, GL_COMPUTE_SHADER},
};

spp::sparse_hash_map<ShaderType, std::string> const FROM_SHADER_TYPE_TO_HUMAN_STRING{
    {VERTEX_SHADER, "vertex shader"},
    {FRAGMENT_SHADER, "fragment shader"},
    {COMPUTE_SHADER, "compute shader"},
};

types::UInt compileShader(char const *shaderSource, ShaderType shaderType);

types::UInt linkProgram(std::vector<types::UInt> const &shaderIds);

Shader::Shader()
    : _id(0)
{
}

Shader::Shader(std::shared_ptr<Window> window,
               std::filesystem::path const &vertexShaderPath,
               std::filesystem::path const &fragmentShaderPath)
    : _window(std::move(window))
{
    _window->bindContext();

    std::string vertexShaderSource = pf::util::file::readAsText(vertexShaderPath);
    std::string fragmentShaderSource = pf::util::file::readAsText(fragmentShaderPath);
    types::UInt vertexShader = compileShader(vertexShaderSource.c_str(), VERTEX_SHADER);
    types::UInt fragmentShader = compileShader(fragmentShaderSource.c_str(), FRAGMENT_SHADER);
    _id = linkProgram({vertexShader, fragmentShader});
}

Shader::Shader(std::shared_ptr<Window> window,
               std::filesystem::path const &shaderPath,
               ShaderType shaderType)
    : _window(std::move(window))
{
    if (FROM_SHADER_TYPE_TO_GL_ENUM.find(shaderType) == FROM_SHADER_TYPE_TO_GL_ENUM.end())
    {
        throw std::invalid_argument("Specified shader type is not supported.");
    }

    _window->bindContext();

    std::string shaderSource = pf::util::file::readAsText(shaderPath);
    types::UInt shader = compileShader(shaderSource.c_str(), shaderType);
    _id = linkProgram({shader});
}

void Shader::use() const
{
    if (_id == 0)
    {
        throw std::runtime_error("Cannot use the dummy shader.");
    }

    _window->bindContext();
    glUseProgram(_id);
}

template <>
void Shader::setUniformValue<types::Float>(char const *name, types::Float value)
{
    // getUniformLocation throws an exception in case the uniform is not found
    types::Int uniformLocation = getUniformLocation(name);
    glUniform1f(uniformLocation, value);
}

template <>
void Shader::setUniformValue<types::FVec3>(char const *name, types::FVec3 value)
{
    GLint uniformLocation = getUniformLocation(name);
    glUniform3f(uniformLocation, value.x, value.y, value.z);
}

template <>
void Shader::setUniformValue<types::FVec2>(char const *name, types::FVec2 value)
{
    GLint uniformLocation = getUniformLocation(name);
    glUniform2f(uniformLocation, value.x, value.y);
}

template <>
void Shader::setUniformValue<types::FMat4>(char const *name, types::FMat4 value)
{
    GLint uniformLocation = getUniformLocation(name);
    glUniformMatrix4fv(
        uniformLocation, 1, GL_FALSE, types::dataPointer<types::FMat4, types::Float>(value));
}

template <>
void Shader::setUniformValue<types::IntVec2>(char const *name, types::IntVec2 value)
{
    GLint uniformLocation = getUniformLocation(name);
    glUniform2i(uniformLocation, value.x, value.y);
}

template <>
void Shader::setUniformValue<types::Int>(char const *name, types::Int value)
{
    GLint uniformLocation = getUniformLocation(name);
    glUniform1i(uniformLocation, value);
}

template <>
void Shader::setUniformValue<types::Bool>(char const *name, types::Bool value)
{
    GLint uniformLocation = getUniformLocation(name);
    glUniform1i(uniformLocation, value);
}

types::Int Shader::getUniformLocation(char const *name)
{
    if (_id == 0)
    {
        throw std::runtime_error("Cannot set the uniform value for the dummy shader.");
    }

    _window->bindContext();
    this->use();
    types::Int uniformLocation = glGetUniformLocation(_id, name);
    if (uniformLocation == -1)
    {
        throw std::invalid_argument(fmt::format("Cannot find uniform \"{}\".", name));
    }

    return uniformLocation;
}

void Shader::unbind() const
{
    if (_id == 0)
    {
        throw std::runtime_error("Cannot unbind the dummy shader.");
    }

    _window->bindContext();
    glUseProgram(0);
}

Shader::~Shader()
{
    if (_id == 0)
    {
        return;
    }

    _window->bindContext();
    glDeleteProgram(_id);
}

GLuint compileShader(char const *shaderSource, ShaderType shaderType)
{
    types::UInt shaderId = glCreateShader(FROM_SHADER_TYPE_TO_GL_ENUM.at(shaderType));
    if (shaderId == 0)
    {
        throw std::runtime_error("Failed to create a shader.");
    }

    glShaderSource(shaderId, 1, &shaderSource, nullptr);
    glCompileShader(shaderId);

    types::Int hasCompiled = GL_FALSE;
    types::Int infoLogLength = 0;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &hasCompiled);
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);

    if (hasCompiled == GL_FALSE)
    {
        std::string infoLog(infoLogLength, ' ');
        glGetShaderInfoLog(shaderId, infoLogLength, nullptr, infoLog.data());
        glDeleteShader(shaderId);

        throw std::runtime_error(fmt::format(
            "{} compilation error: {}.", FROM_SHADER_TYPE_TO_HUMAN_STRING.at(shaderType), infoLog));
    }

    return shaderId;
}

GLuint linkProgram(std::vector<types::UInt> const &shaderIds)
{
    GLuint programId = glCreateProgram();
    if (programId == 0)
    {
        throw std::runtime_error("Failed to create a program");
    }

    for (types::UInt shader : shaderIds)
    {
        glAttachShader(programId, shader);
    }

    glLinkProgram(programId);

    types::Int hasLinked = GL_FALSE;
    types::Int infoLogLength = 0;
    glGetProgramiv(programId, GL_LINK_STATUS, &hasLinked);
    for (types::UInt shaderId : shaderIds)
    {
        glDeleteShader(shaderId);
    }

    glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (hasLinked == 0)
    {
        std::string infoLog(infoLogLength, ' ');
        glGetProgramInfoLog(programId, infoLogLength, nullptr, infoLog.data());
        glDeleteProgram(programId);

        throw std::runtime_error(fmt::format("Program linking error: {}.", infoLog));
    }

    return programId;
}

} // namespace pf::gl
