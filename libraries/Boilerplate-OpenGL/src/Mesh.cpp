#include <pf_gl/Mesh.hpp>

#include <cstddef>
#include <utility>
#include <memory>
#include <vector>
#include <string>
#include <span>

#include <gsl/util>

#include <pf_gl/VertexBuffer.hpp>
#include <pf_gl/ElementBuffer.hpp>
#include <pf_gl/Texture.hpp>
#include <pf_gl/Transform3D.hpp>
#include <pf_gl/MinecraftCamera.hpp>
#include <pf_gl/VertexLayout.hpp>
#include <pf_gl/ValueTypes.hpp>
#include <pf_gl/RenderingOptions.hpp>
#include <pf_gl/Window.hpp>
#include <pf_utils/RawBuffer.hpp>
#include <pf_gl/ValueTypes.hpp>
#include <pf_gl/DrawingContext3D.hpp>
#include <pf_gl/EulerTransform3D.hpp>

namespace pf::gl
{

Mesh::Mesh(std::shared_ptr<Window> window,
           std::vector<SimpleVertex> const &vertices,
           std::vector<GLuint> const &indices,
           std::vector<std::shared_ptr<Texture>> textures,
           UsagePattern usagePattern)
    : _window(std::move(window))
    , _textures(std::move(textures))
{
    _vertexArray = std::make_shared<VertexArray>(_window);

    auto vertexBuffer = std::make_shared<VertexBuffer>(
        _window,
        pf::util::RawBuffer(vertices),
        usagePattern,
        VertexLayout({
            AttributeEntry(types::FLOAT_VECTOR_3, POSITION),
            AttributeEntry(types::FLOAT_VECTOR_3, NORMAL),
            AttributeEntry(types::FLOAT_VECTOR_2, TEXTURE_COORDINATES),
        }));
    _vertexArray->addVertexBuffer(vertexBuffer);

    auto elementBuffer = std::make_shared<ElementBuffer>(
        _window, std::span<const types::UInt>(indices.begin(), indices.end()), usagePattern);
    _vertexArray->setElementBuffer(elementBuffer);
}

Mesh::Mesh(std::shared_ptr<Window> window,
           pf::util::RawBuffer const &vertices,
           VertexLayout const &vertexLayout,
           std::vector<GLuint> &indices,
           std::vector<std::shared_ptr<Texture>> textures,
           UsagePattern usagePattern)
    : _window(std::move(window))
    , _textures(std::move(textures))
{
    _vertexArray = std::make_shared<VertexArray>(_window);

    auto vertexBuffer =
        std::make_shared<VertexBuffer>(_window, vertices, usagePattern, vertexLayout);
    _vertexArray->addVertexBuffer(vertexBuffer);

    auto elementBuffer = std::make_shared<ElementBuffer>(
        _window, std::span<types::UInt>(indices.begin(), indices.size()), usagePattern);
    _vertexArray->setElementBuffer(elementBuffer);
}

void Mesh::render(Shader &shader,
                  DrawingContext3D const &drawingContext,
                  Transform3D const &transform,
                  Material const &material) const
{
    shader.use();

    types::Int textureIndex = 0;

    for (auto const &uniform : shader.uniforms())
    {
        switch (uniform.purpose)
        {
        case Uniform::Purpose::VIEW_MATRIX:
        {
            if (!drawingContext.camera.has_value())
            {
                break;
            }

            types::FMat4 viewMatrix = drawingContext.camera->viewMatrix();
            glUniformMatrix4fv(uniform.location(),
                               1,
                               GL_FALSE,
                               types::dataPointer<types::FMat4, types::Float>(viewMatrix));
            break;
        }

        case Uniform::Purpose::PROJECTION_MATRIX:
        {
            if (!drawingContext.camera.has_value())
            {
                break;
            }

            types::FMat4 projectionMatrix = drawingContext.camera->projectionMatrix();
            glUniformMatrix4fv(uniform.location(),
                               1,
                               GL_FALSE,
                               types::dataPointer<types::FMat4, types::Float>(projectionMatrix));
            break;
        }

        case Uniform::Purpose::MODEL_MATRIX:
        {
            types::FMat4 modelMatrix = transform.localToWorldMatrix();
            glUniformMatrix4fv(uniform.location(),
                               1,
                               GL_FALSE,
                               types::dataPointer<types::FMat4, types::Float>(modelMatrix));
            break;
        }

        case Uniform::Purpose::DIFFUSE_TEXTURE:
        case Uniform::Purpose::SPECULAR_TEXTURE:
        {
            if (textureIndex >= _textures.size())
            {
                break;
            }

            glActiveTexture(GL_TEXTURE0 + textureIndex);
            _textures[textureIndex]->bind();
            glUniform1i(uniform.location(), textureIndex);
            textureIndex++;
            break;
        }

        case Uniform::Purpose::DIRECTIONAL_LIGHT:
        {
            if (!drawingContext.directionalLight.has_value())
            {
                break;
            }

            DirectionalLight const &directionalLight = drawingContext.directionalLight.value();
            // count is set to 1, because vec3 is treated like an array of vec3s of size 1
            glUniform3fv(uniform.child("direction").location, 1, &directionalLight.direction[0]);
            glUniform3fv(uniform.child("ambient").location, 1, &directionalLight.color.ambient[0]);
            glUniform3fv(uniform.child("diffuse").location, 1, &directionalLight.color.diffuse[0]);
            glUniform3fv(
                uniform.child("specular").location, 1, &directionalLight.color.specular[0]);
            break;
        }

        case Uniform::Purpose::DIRECTIONAL_LIGHT_ENABLED:
        {
            glUniform1i(uniform.location(), drawingContext.directionalLight.has_value() ? 1 : 0);
            break;
        }

        case Uniform::Purpose::POINT_LIGHT:
        {
            if (!drawingContext.pointLights.has_value() || uniform.arrayIndex < 0 ||
                drawingContext.pointLights->size() <= uniform.arrayIndex)
            {
                break;
            }

            PointLight const &pointLight = drawingContext.pointLights.value()[uniform.arrayIndex];

            types::FVec3 positionFromViewer = pointLight.position;
            if (drawingContext.camera.has_value())
            {
                positionFromViewer =
                    drawingContext.camera->viewMatrix() * types::FVec4(positionFromViewer, 1.0F);
            }

            glUniform3fv(uniform.child("position").location, 1, &positionFromViewer[0]);
            glUniform3fv(uniform.child("ambient").location, 1, &pointLight.color.ambient[0]);
            glUniform3fv(uniform.child("diffuse").location, 1, &pointLight.color.diffuse[0]);
            glUniform3fv(uniform.child("specular").location, 1, &pointLight.color.specular[0]);
            glUniform1f(uniform.child("constantFactor").location, pointLight.falloff.constant);
            glUniform1f(uniform.child("linearFactor").location, pointLight.falloff.linear);
            glUniform1f(uniform.child("quadraticFactor").location, pointLight.falloff.quadratic);
            break;
        }

        case Uniform::Purpose::POINT_LIGHTS_COUNT:
        {
            if (!drawingContext.pointLights.has_value())
            {
                glUniform1i(uniform.location(), 0);
                break;
            }

            glUniform1i(uniform.location(),
                        gsl::narrow_cast<types::Int>(drawingContext.pointLights->size()));
            break;
        }

        case Uniform::Purpose::SPOT_LIGHT:
        {
            if (!drawingContext.spotLight.has_value())
            {
                break;
            }

            SpotLight const &spotLight = drawingContext.spotLight.value();
            glUniform3fv(uniform.child("position").location, 1, &spotLight.position[0]);
            glUniform3fv(uniform.child("direction").location, 1, &spotLight.direction[0]);
            glUniform3fv(uniform.child("ambient").location, 1, &spotLight.color.ambient[0]);
            glUniform3fv(uniform.child("diffuse").location, 1, &spotLight.color.diffuse[0]);
            glUniform3fv(uniform.child("specular").location, 1, &spotLight.color.specular[0]);
            glUniform1f(uniform.child("constantFactor").location, spotLight.falloff.constant);
            glUniform1f(uniform.child("linearFactor").location, spotLight.falloff.linear);
            glUniform1f(uniform.child("quadraticFactor").location, spotLight.falloff.quadratic);
            glUniform1f(uniform.child("cosCutOff").location, spotLight.cutoff);
            glUniform1f(uniform.child("cosOuterCutOff").location, spotLight.outerCutoff);
            break;
        }

        case Uniform::Purpose::SPOT_LIGHT_ENABLED:
        {
            glUniform1i(uniform.location(), drawingContext.spotLight.has_value() ? 1 : 0);
            break;
        };

        case Uniform::Purpose::SHININESS:
        {
            glUniform1f(uniform.location(), material.shininess);
            break;
        }

        case Uniform::Purpose::COLOR:
        {
            glUniform3fv(uniform.location(), 1, &material.color[0]);
            break;
        }

        case Uniform::Purpose::ELAPSED_TIME_SECONDS:
        {
            if (!drawingContext.elapsedTimeSeconds.has_value())
            {
                break;
            }

            glUniform1f(uniform.location(), drawingContext.elapsedTimeSeconds.value());
            break;
        }

        case Uniform::Purpose::VIEWPORT_SIZE:
        {
            if (!drawingContext.viewportSize.has_value())
            {
                break;
            }

            glUniform2fv(uniform.location(), 1, &drawingContext.viewportSize.value()[0]);
            break;
        }
        }
    }
    _vertexArray->draw();
}

void Mesh::render(Shader &shader,
                  DrawingContext3D const &drawingContext,
                  Material const &material) const
{
    render(shader, drawingContext, EulerTransform3D::IDENTITY, material);
}

} // namespace pf::gl
