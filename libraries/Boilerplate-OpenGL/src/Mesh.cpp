#include <pf_gl/Mesh.hpp>

#include <cstddef>
#include <utility>
#include <memory>
#include <vector>
#include <string>
#include <span>

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

namespace pf::gl
{

pf::util::IndexedString Mesh::DIFFUSE_UNIFORM_NAME = "u_textureDiffuse???";   // NOLINT
pf::util::IndexedString Mesh::SPECULAR_UNIFORM_NAME = "u_textureSpecular???"; // NOLINT

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

void Mesh::render(Shader &shader, MinecraftCamera const &camera, Transform3D const &transform) const
{
    shader.use();
    activateTextures(shader);

    // setup matrices
    shader.setUniformValue("u_view", camera.viewMatrix());
    shader.setUniformValue("u_projection", camera.projectionMatrix());
    shader.setUniformValue("u_model", transform.localToWorldMatrix());

    _vertexArray->draw();
}

void Mesh::render(Shader &shader) const
{
    shader.use();
    activateTextures(shader);

    _vertexArray->draw();
}

void Mesh::activateTextures(Shader &shader) const
{
    size_t diffuseTextureIndex = 0;
    size_t specularTextureIndex = 0;

    for (types::Int i = 0; i < _textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        _textures[i]->bind();

        char const *uniformName = nullptr;
        switch (_textures[i]->type())
        {
        case TextureType::DIFFUSE:
            uniformName = DIFFUSE_UNIFORM_NAME.withIndex(diffuseTextureIndex++);
            break;
        case TextureType::SPECULAR:
            uniformName = SPECULAR_UNIFORM_NAME.withIndex(specularTextureIndex++);
            break;
        }
        if (uniformName != nullptr)
        {
            shader.setUniformValue(uniformName, i);
        }
    }
}

} // namespace pf::gl
