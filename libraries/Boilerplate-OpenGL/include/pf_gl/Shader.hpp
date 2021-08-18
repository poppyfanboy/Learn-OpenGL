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
#include <pf_utils/Hashing.hpp>

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
        SPOT_LIGHT_POSITION,
        SPOT_LIGHT_DIRECTION,
        SPOT_LIGHT_CUTOFF,
        SPOT_LIGHT_OUTER_CUTOFF,
        SPOT_LIGHT_AMBIENT,
        SPOT_LIGHT_DIFFUSE,
        SPOT_LIGHT_SPECULAR,
        SPOT_LIGHT_CONSTANT_FACTOR,
        SPOT_LIGHT_LINEAR_FACTOR,
        SPOT_LIGHT_QUADRATIC_FACTOR,

        DIRECTIONAL_LIGHT,
        DIRECTIONAL_LIGHT_ENABLED,
        DIRECTIONAL_LIGHT_DIRECTION,
        DIRECTIONAL_LIGHT_AMBIENT,
        DIRECTIONAL_LIGHT_DIFFUSE,
        DIRECTIONAL_LIGHT_SPECULAR,

        POINT_LIGHT,
        POINT_LIGHTS_COUNT,
        POINT_LIGHT_POSITION,
        POINT_LIGHT_AMBIENT,
        POINT_LIGHT_DIFFUSE,
        POINT_LIGHT_SPECULAR,
        POINT_LIGHT_CONSTANT_FACTOR,
        POINT_LIGHT_LINEAR_FACTOR,
        POINT_LIGHT_QUADRATIC_FACTOR,

        GENERIC,
    };

    static spp::sparse_hash_map<std::string, Purpose> const NAME_TO_PURPOSE;
    static spp::sparse_hash_map<std::pair<Uniform::Purpose, std::string>,
                                Uniform::Purpose,
                                pf::util::PairHash> const CONTEXT_NAME_TO_PURPOSE;

    std::string fullName;
    // everything before the first period in the full name, also the array index is removed
    std::string baseName;
    // everything after the first period in the full name
    std::string secondaryName;

    // So, if fullName = "first[2].second.third", then baseName = "first" and secondaryName =
    // "second.third"

    // In case the base name of the uniform name has an array index it is stored here
    types::Int arrayIndex = -1;

    types::Int location;
    Purpose purpose = GENERIC;
};

} // namespace pf::gl

#endif // !SHADER_HPP
