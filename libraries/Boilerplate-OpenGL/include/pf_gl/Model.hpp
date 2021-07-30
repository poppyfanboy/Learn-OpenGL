#ifndef MODEL_HPP
#define MODEL_HPP

#include <memory>
#include <string>
#include <vector>
#include <filesystem>

#include <assimp/scene.h>

#include <pf_gl/Shader.hpp>
#include <pf_gl/MinecraftCamera.hpp>
#include <pf_gl/Mesh.hpp>
#include <pf_gl/Transform3D.hpp>
#include <pf_gl/EulerTransform3D.hpp>
#include <pf_gl/Texture.hpp>
#include <pf_gl/Window.hpp>

namespace pf::gl
{

class Model
{
public:
    /**
     * Load model from disk.
     */
    Model(std::shared_ptr<Window> window,
          std::filesystem::path const &path,
          std::unique_ptr<Transform3D> &&transform = std::make_unique<EulerTransform3D>());

    /**
     * Create model from a collection of meshes.
     */
    Model(std::shared_ptr<Window> window,
          std::vector<std::shared_ptr<Mesh>> meshes,
          std::unique_ptr<Transform3D> &&transform = std::make_unique<EulerTransform3D>());

    [[nodiscard]] Transform3D const &transform() const;

    void render(Shader &shader, MinecraftCamera const &camera) const;

    void transform(std::unique_ptr<Transform3D> &&transform);

private:
    std::shared_ptr<Window> _window;
    std::vector<std::shared_ptr<Mesh>> _meshes;
    std::vector<std::shared_ptr<Texture>> _loadedTextures;
    std::unique_ptr<Transform3D> _transform;

    void loadModel(std::filesystem::path const &modelPath);

    /**
     * In assimp each scene (the complete model) is a tree-like structure of nodes.  Each node can
     * have multiple meshes. Going from the root node first we process every mesh belonging to the
     * node, and then process all children nodes in a recursive manner.
     *
     * Pointers to meshes are only stored inside the scene as an array. Nodes refer to meshes by
     * their indices. Same thing with materials: meshes store materials as indices of the array in
     * the scene.
     */
    void processNode(aiNode *node, aiScene const *scene, std::filesystem::path const &modelPath);

    /**
     * Converts the model from assimp format to my custom object for meshes. May return a nullptr in
     * case assimp parser returns a mesh with no vertices or something else goes wrong.
     */
    std::unique_ptr<Mesh>
    processMesh(aiMesh *mesh, aiScene const *scene, std::filesystem::path const &modelPath);

    /**
     * Loads textures assigned to the material from the disk.
     */
    std::vector<std::shared_ptr<Texture>>
    loadMaterialTextures(aiMaterial *material,
                         aiTextureType type,
                         TextureType textureType,
                         std::filesystem::path const &modelPath);
};

} // namespace pf::gl

#endif // !MODEL_HPP
