#ifndef SHADER_HPP
#define SHADER_HPP

#include <memory>
#include <filesystem>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>
#include <stdexcept>

#include <sparsepp/spp.h>

#include <pf_gl/Window.hpp>
#include <pf_gl/ValueTypes.hpp>

namespace pf::gl
{

struct Uniform;

class Shader final
{
public:
    enum Type
    {
        VERTEX_SHADER,
        FRAGMENT_SHADER,
        COMPUTE_SHADER,
    };

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
           Type shaderType);

    ~Shader();

    Shader &operator=(Shader const &) = delete;
    Shader &operator=(Shader &&) = default;

    void use() const;
    void unbind() const;

    [[nodiscard]] types::UInt id() const;
    [[nodiscard]] std::span<Uniform const> uniforms() const;

    void setUniformValue(char const *name, types::Float value);
    void setUniformValue(char const *name, types::FVec3 value);
    void setUniformValue(char const *name, types::FVec2 value);
    void setUniformValue(char const *name, types::FMat4 value);
    void setUniformValue(char const *name, types::IntVec2 value);
    void setUniformValue(char const *name, types::Int value);
    void setUniformValue(char const *name, types::Bool value);

private:
    types::UInt _id;
    std::shared_ptr<Window> _window;
    std::vector<Uniform> _uniforms;

    GLint getUniformLocation(char const *name);
    GLuint compileShader(char const *shaderSource, Type shaderType);
    GLuint linkProgram(std::vector<types::UInt> const &shaderIds);
    void retrieveUniforms();
};

struct ScalarUniform
{
    std::string fullName;
    std::string name;
    types::Int location;
};

/**
 * For simplicity sake this uniform could be a struct, an array [of structs] or just a saclar value
 */
struct Uniform
{
    enum Purpose
    {
        ELAPSED_TIME_SECONDS,
        VIEWPORT_SIZE,
        MODEL_MATRIX,
        VIEW_MATRIX,
        PROJECTION_MATRIX,
        DIFFUSE_TEXTURE,
        SPECULAR_TEXTURE,
        SHININESS,
        COLOR,
        SPOT_LIGHT,
        SPOT_LIGHT_ENABLED,
        DIRECTIONAL_LIGHT,
        DIRECTIONAL_LIGHT_ENABLED,
        POINT_LIGHT,
        POINT_LIGHTS_COUNT,
        GENERIC,
    };

    static spp::sparse_hash_map<std::string, Purpose> const NAME_TO_PURPOSE;

    std::string baseName;
    Purpose purpose = GENERIC;
    // for array-uniform usage case
    types::Int arrayIndex = -1;
    spp::sparse_hash_map<std::string, ScalarUniform> children;

    // convenience methods for scalar uniform usage case
    [[nodiscard]] types::Int location() const;
    [[nodiscard]] std::string const &name() const;

    // for struct-uniform usage case
    ScalarUniform const &child(char const *name) const;
};

} // namespace pf::gl

#endif // !SHADER_HPP
