#ifndef DRAWING_CONTEXT3D_HPP
#define DRAWING_CONTEXT3D_HPP

#include <memory>
#include <string>
#include <vector>
#include <span>
#include <optional>
#include <any>

#include <sparsepp/spp.h>

#include <pf_gl/MinecraftCamera.hpp>
#include <pf_gl/Transform3D.hpp>
#include <pf_gl/ValueTypes.hpp>

namespace pf::gl
{

struct LightColor
{
    types::FVec3 ambient;
    types::FVec3 diffuse;
    types::FVec3 specular;
};

struct LightFalloff
{
    types::Float constant;
    types::Float linear;
    types::Float quadratic;
};

struct DirectionalLight
{
    types::FVec3 direction;
    LightColor color;
};

struct PointLight
{
    types::FVec3 position;
    LightColor color;
    LightFalloff falloff;
};

struct SpotLight
{
    types::FVec3 position;
    types::FVec3 direction;
    LightColor color;
    LightFalloff falloff;
    types::Float cutoff;
    types::Float outerCutoff;
};

struct DrawingContext3D
{
    // TODO(poppyfanboy) This shouldn't be here, a proper place would be a material or something
    std::optional<types::Float> shininess;

    std::optional<MinecraftCamera> camera;
    std::optional<types::Float> elapsedTimeSeconds;
    std::optional<types::FVec2> viewportSize;
    std::optional<std::vector<PointLight>> pointLights;
    std::optional<DirectionalLight> directionalLight;
    std::optional<SpotLight> spotLight;

    spp::sparse_hash_map<std::string, std::any> values =
        spp::sparse_hash_map<std::string, std::any>();
};

} // namespace pf::gl

#endif // !DRAWING_CONTEXT3D_HPP
