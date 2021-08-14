#ifndef MESH_HPP
#define MESH_HPP

#include <memory>
#include <vector>

#include <glad/glad.h>

#include <pf_gl/VertexArray.hpp>
#include <pf_gl/Texture.hpp>
#include <pf_gl/Shader.hpp>
#include <pf_gl/ElementBuffer.hpp>
#include <pf_gl/MinecraftCamera.hpp>
#include <pf_gl/Transform3D.hpp>
#include <pf_gl/RenderingOptions.hpp>
#include <pf_gl/Window.hpp>
#include <pf_gl/ValueTypes.hpp>
#include <pf_utils/RawBuffer.hpp>
#include <pf_utils/IndexedString.hpp>
#include <pf_gl/DrawingContext3D.hpp>
#include <pf_gl/Material.hpp>

namespace pf::gl
{

// TODO(poppyfanboy) Add a Mesh abstract class. Add two concrete classes: `StaticMesh` and
// `EditableMesh`. The first one will store just the bare minimum to render itself on screen. The
// second one will have methods for messing with mesh's geometry.

class Mesh final
{
public:
    struct SimpleVertex
    {
        types::FVec3 position;
        types::FVec3 normal;
        types::FVec2 textureCoordinates;
    };

    /**
     * Create mesh from a set of vertices with (position, normal, UV) layout.
     */
    Mesh(std::shared_ptr<Window> window,
         std::vector<SimpleVertex> const &vertices,
         std::vector<GLuint> const &indices,
         std::vector<std::shared_ptr<Texture>> textures,
         UsagePattern usagePattern);

    /**
     * Create mesh from a raw buffer with custom layout.
     */
    Mesh(std::shared_ptr<Window> window,
         pf::util::RawBuffer const &vertices,
         VertexLayout const &vertexLayout,
         std::vector<GLuint> &indices,
         std::vector<std::shared_ptr<Texture>> textures,
         UsagePattern usagePattern);

    void render(Shader &shader,
                DrawingContext3D const &drawingContext,
                Transform3D const &transform,
                Material const &material = {}) const;

    void render(Shader &shader,
                DrawingContext3D const &drawingContext,
                Material const &material = {}) const;

private:
    std::shared_ptr<Window> _window;
    std::vector<std::shared_ptr<Texture>> _textures;
    std::shared_ptr<VertexArray> _vertexArray;
};

} // namespace pf::gl

#endif // !MESH_HPP
