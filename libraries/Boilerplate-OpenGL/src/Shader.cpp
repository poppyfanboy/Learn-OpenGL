#include <pf_gl/Shader.hpp>

#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <stdexcept>
#include <filesystem>
#include <cassert>
#include <iterator>
#include <regex>
#include <span>
#include <ranges>
#include <algorithm>

#include <glad/glad.h>
#include <fmt/format.h>
#include <sparsepp/spp.h>
#include <gsl/util>

#include <pf_gl/Window.hpp>
#include <pf_gl/ValueTypes.hpp>
#include <pf_utils/FileUtils.hpp>

namespace pf::gl
{

spp::sparse_hash_map<Shader::Type, GLenum> const FROM_SHADER_TYPE_TO_GL_ENUM{
    {Shader::VERTEX_SHADER, GL_VERTEX_SHADER},
    {Shader::FRAGMENT_SHADER, GL_FRAGMENT_SHADER},
    {Shader::COMPUTE_SHADER, GL_COMPUTE_SHADER},
};

spp::sparse_hash_map<Shader::Type, std::string> const FROM_SHADER_TYPE_TO_HUMAN_STRING{
    {Shader::VERTEX_SHADER, "vertex shader"},
    {Shader::FRAGMENT_SHADER, "fragment shader"},
    {Shader::COMPUTE_SHADER, "compute shader"},
};

types::UInt compileShader(char const *shaderSource, Shader::Type shaderType);

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

    retrieveUniforms();
}

Shader::Shader(std::shared_ptr<Window> window,
               std::filesystem::path const &shaderPath,
               Type shaderType)
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

    retrieveUniforms();
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

void Shader::setUniformValue(char const *name, types::Float value)
{
    // getUniformLocation throws an exception in case the uniform is not found
    types::Int uniformLocation = getUniformLocation(name);
    glUniform1f(uniformLocation, value);
}

void Shader::setUniformValue(char const *name, types::FVec3 value)
{
    GLint uniformLocation = getUniformLocation(name);
    glUniform3f(uniformLocation, value.x, value.y, value.z);
}

void Shader::setUniformValue(char const *name, types::FVec2 value)
{
    GLint uniformLocation = getUniformLocation(name);
    glUniform2f(uniformLocation, value.x, value.y);
}

void Shader::setUniformValue(char const *name, types::FMat4 value)
{
    GLint uniformLocation = getUniformLocation(name);
    glUniformMatrix4fv(
        uniformLocation, 1, GL_FALSE, types::dataPointer<types::FMat4, types::Float>(value));
}

void Shader::setUniformValue(char const *name, types::IntVec2 value)
{
    GLint uniformLocation = getUniformLocation(name);
    glUniform2i(uniformLocation, value.x, value.y);
}

void Shader::setUniformValue(char const *name, types::Int value)
{
    GLint uniformLocation = getUniformLocation(name);
    glUniform1i(uniformLocation, value);
}

void Shader::setUniformValue(char const *name, types::Bool value)
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

spp::sparse_hash_map<std::string, Uniform::Purpose> const Uniform::NAME_TO_PURPOSE{
    {"u_time", ELAPSED_TIME_SECONDS},
    {"u_resolution", VIEWPORT_SIZE},
    {"u_model", MODEL_MATRIX},
    {"u_view", VIEW_MATRIX},
    {"u_projection", PROJECTION_MATRIX},
    {"u_diffuseTexture", DIFFUSE_TEXTURE},
    {"u_specularTexture", SPECULAR_TEXTURE},
    {"u_shininess", SHININESS},
    {"u_spotLight", SPOT_LIGHT},
    {"u_spotLightEnabled", SPOT_LIGHT_ENABLED},
    {"u_directionalLight", DIRECTIONAL_LIGHT},
    {"u_directionalLightEnabled", DIRECTIONAL_LIGHT_ENABLED},
    {"u_pointLights", POINT_LIGHT},
    {"u_pointLightsCount", POINT_LIGHTS_COUNT},
    {"u_color", COLOR},
};

std::regex const IS_ARRAY(R"(^(\w+)\[(\d+)\]$)");
std::regex const SPLIT_BY_PERIOD(R"(\.)");

void Shader::retrieveUniforms()
{
    assert(_id != 0 && _window != nullptr);

    types::Int uniformsCount = 0;
    glGetProgramiv(_id, GL_ACTIVE_UNIFORMS, &uniformsCount);

    for (types::Int uniformIndex = 0; uniformIndex < uniformsCount; uniformIndex++)
    {
        std::string rawUniformName(GL_ACTIVE_UNIFORM_MAX_LENGTH, ' ');
        GLenum uniformType = 0;
        types::Int uniformSize = 0;

        glGetActiveUniform(_id,
                           uniformIndex,
                           gsl::narrow_cast<types::Size>(rawUniformName.size()),
                           nullptr,
                           &uniformSize,
                           &uniformType,
                           &rawUniformName[0]);

        // if rawUniformName = "first.second.last" iterates over "first", "second", "last"
        std::sregex_token_iterator periodSeparatedFirst{
            rawUniformName.begin(), rawUniformName.end(), SPLIT_BY_PERIOD, -1};
        std::string rawBaseName = periodSeparatedFirst->str().c_str();

        // get the base name and an array index to find if this uniform has already been added

        std::string uniformName;
        std::string::size_type firstDotPosition = rawUniformName.find('.', 0);
        if (firstDotPosition == std::string::npos)
        {
            uniformName = rawUniformName.c_str();
        }
        else
        {
            uniformName = &rawUniformName[firstDotPosition + 1];
        }

        std::string uniformBaseName;
        types::Int uniformArrayIndex = -1;

        std::smatch isArrayMatch;
        if (std::regex_search(rawBaseName, isArrayMatch, IS_ARRAY))
        {
            uniformBaseName = isArrayMatch.str(1);
            uniformArrayIndex = std::stoi(isArrayMatch.str(2));
        }
        else
        {
            uniformBaseName = rawBaseName;
            uniformArrayIndex = -1;
        }

        auto foundUniform =
            std::ranges::find_if(_uniforms,
                                 [uniformBaseName, uniformArrayIndex](Uniform const &uniform) {
                                     return uniform.arrayIndex == uniformArrayIndex &&
                                            uniform.baseName == uniformBaseName;
                                 });

        ScalarUniform singularUniform = {
            .fullName = rawUniformName.c_str(),
            .name = uniformName,
            .location = getUniformLocation(rawUniformName.c_str()),
        };
        if (foundUniform != _uniforms.end())
        {
            foundUniform->children[singularUniform.name] = singularUniform;
        }
        else
        {
            Uniform::Purpose uniformPurpose = Uniform::GENERIC;
            if (Uniform::NAME_TO_PURPOSE.find(uniformBaseName) != Uniform::NAME_TO_PURPOSE.end())
            {
                uniformPurpose = Uniform::NAME_TO_PURPOSE.at(uniformBaseName);
            }

            Uniform uniform = {
                .baseName = uniformBaseName,
                .purpose = uniformPurpose,
                .arrayIndex = uniformArrayIndex,
                .children = {{singularUniform.name, singularUniform}},
            };
            _uniforms.push_back(std::move(uniform));
        }
    }
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

[[nodiscard]] types::UInt Shader::id() const
{
    return _id;
}

[[nodiscard]] std::span<Uniform const> Shader::uniforms() const
{
    return _uniforms;
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

GLuint Shader::compileShader(char const *shaderSource, Type shaderType)
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

GLuint Shader::linkProgram(std::vector<types::UInt> const &shaderIds)
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

[[nodiscard]] types::Int Uniform::location() const
{
    return children.begin()->second.location;
}

[[nodiscard]] std::string const &Uniform::name() const
{
    return children.begin()->second.fullName;
}

ScalarUniform const &Uniform::child(char const *name) const
{
    auto child = children.find(name);
    if (child == children.end())
    {
        throw std::invalid_argument("Cannot find the singular uniform");
    }
    return child->second;
}

} // namespace pf::gl
