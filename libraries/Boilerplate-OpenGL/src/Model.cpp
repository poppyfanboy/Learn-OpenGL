#include <map>
#include <numeric>
#include <pf_gl/Model.hpp>

#include <stdexcept>
#include <utility>
#include <memory>
#include <sstream>
#include <algorithm>
#include <filesystem>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <assimp/types.h>
#include <glad/glad.h>
#include <fmt/format.h>

#include <pf_gl/Mesh.hpp>
#include <pf_gl/Texture.hpp>
#include <pf_gl/RenderingOptions.hpp>
#include <pf_gl/Window.hpp>
#include <pf_gl/Transform3D.hpp>
#include <pf_gl/ValueTypes.hpp>
#include <pf_gl/DrawingContext3D.hpp>
#include <pf_gl/Material.hpp>

namespace pf::gl
{

Model::Model(std::shared_ptr<Window> window,
             std::filesystem::path const &path,
             std::unique_ptr<Transform3D> &&transform,
             Material const &material)
    : _window(std::move(window))
    , _transform(std::move(transform))
    , _material(material)
{
    loadModel(path);
}

Model::Model(std::shared_ptr<Window> window,
             std::vector<std::shared_ptr<Mesh>> meshes,
             std::unique_ptr<Transform3D> &&transform,
             Material const &material)
    : _window(std::move(window))
    , _transform(std::move(transform))
    , _meshes(std::move(meshes))
    , _material(material)
{
}

void Model::render(Shader &shader, DrawingContext3D const &drawingContext) const
{
    for (auto const &mesh : _meshes)
    {
        mesh->render(shader, drawingContext, *_transform, _material);
    }
}

void Model::transform(std::unique_ptr<Transform3D> &&transform)
{
    _transform = std::move(transform);
}

Transform3D const &Model::transform() const
{
    return *_transform;
}


void Model::loadModel(std::filesystem::path const &modelPath)
{
    Assimp::Importer importer;
    aiScene const *scene =
        importer.ReadFile(modelPath.string(), aiProcess_Triangulate | aiProcess_FlipUVs);

    if (scene == nullptr || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0U ||
        scene->mRootNode == nullptr)
    {
        throw std::runtime_error(
            fmt::format("Error while loading model using assimp ({}).", importer.GetErrorString()));
    }
    processNode(scene->mRootNode, scene, modelPath);
}

void Model::processNode(aiNode *node, aiScene const *scene, std::filesystem::path const &modelPath)
{
    if (node == nullptr)
    {
        throw std::invalid_argument("Nullptr passed as a node pointer");
    }
    if (scene == nullptr)
    {
        throw std::invalid_argument("Nullptr passed as a scene pointer");
    }

    if (node->mMeshes != nullptr)
    {
        for (types::UInt meshNodeIndex = 0; meshNodeIndex < node->mNumMeshes; meshNodeIndex++)
        {
            types::UInt meshSceneIndex = node->mMeshes[meshNodeIndex];

            if (meshSceneIndex >= scene->mNumMeshes)
            {
                throw std::logic_error("A node points to the mesh with an invalid index.");
            }

            aiMesh *assimpMesh = scene->mMeshes[meshSceneIndex];
            auto mesh = processMesh(assimpMesh, scene, modelPath);
            if (mesh != nullptr)
            {
                _meshes.emplace_back(std::move(mesh));
            }
        }
    }

    if (node->mChildren != nullptr)
    {
        for (unsigned int childIndex = 0; childIndex < node->mNumChildren; childIndex++)
        {
            processNode(node->mChildren[childIndex], scene, modelPath);
        }
    }
}

std::unique_ptr<Mesh>
Model::processMesh(aiMesh *mesh, aiScene const *scene, std::filesystem::path const &modelPath)
{
    if (mesh == nullptr)
    {
        throw std::invalid_argument("Nullptr passed as a pointer to mesh.");
    }
    if (scene == nullptr)
    {
        throw std::invalid_argument("Nullptr passed as a pointer to scene.");
    }

    std::vector<Mesh::SimpleVertex> vertices;
    std::vector<GLuint> indices;
    std::vector<std::shared_ptr<Texture>> textures;

    if (mesh->mVertices == nullptr || mesh->mNumVertices == 0)
    {
        return nullptr;
    }

    for (types::UInt vertexIndex = 0; vertexIndex < mesh->mNumVertices; vertexIndex++)
    {
        Mesh::SimpleVertex vertex{};
        vertex.position = types::FVec3(mesh->mVertices[vertexIndex].x,
                                       mesh->mVertices[vertexIndex].y,
                                       mesh->mVertices[vertexIndex].z);

        vertex.normal = mesh->mNormals != nullptr ? types::FVec3(mesh->mNormals[vertexIndex].x,
                                                                 mesh->mNormals[vertexIndex].y,
                                                                 mesh->mNormals[vertexIndex].z)
                                                  : types::DEFAULT_VALUE<types::FVec3>;

        // Mesh can contain several UV-coordinates, check if the first one is present.
        if (mesh->mTextureCoords[0] != nullptr)
        {
            vertex.textureCoordinates = types::FVec2(mesh->mTextureCoords[0][vertexIndex].x,
                                                     mesh->mTextureCoords[0][vertexIndex].y);
        }
        else
        {
            vertex.textureCoordinates = types::DEFAULT_VALUE<types::FVec2>;
        }

        vertices.push_back(vertex);
    }

    // Information about indices is stored in the assimp's Face abstraction. In case assimp does not
    // return any indices, just create a redundant array of numbers from 1 to N.
    if (mesh->mFaces != nullptr && mesh->mNumFaces != 0)
    {
        for (types::UInt faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++)
        {
            aiFace face = mesh->mFaces[faceIndex];
            for (unsigned int i = 0; i < face.mNumIndices; i++)
            {
                indices.push_back(face.mIndices[i]);
            }
        }
    }
    else
    {
        indices = std::vector<types::UInt>(mesh->mNumVertices);
        std::iota(indices.begin(), indices.end(), 0);
    }

    // Material loading. Every mesh uses exactly one material, if it's not the case, assimp
    // automatically splits meshes so that each of them would have a single material.
    types::UInt materialIndex = mesh->mMaterialIndex;
    if (materialIndex >= scene->mNumMaterials)
    {
        throw std::logic_error("Mesh points to the material that does not exist");
    }

    aiMaterial *material = scene->mMaterials[materialIndex];
    if (material == nullptr)
    {
        throw std::logic_error("Scene has a material stored as a nullptr.");
    }

    // Only diffuse and specular maps are supported for now
    std::vector<std::shared_ptr<Texture>> diffuseMaps =
        loadMaterialTextures(material, aiTextureType_DIFFUSE, TextureType::DIFFUSE, modelPath);
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    std::vector<std::shared_ptr<Texture>> specularMaps =
        loadMaterialTextures(material, aiTextureType_SPECULAR, TextureType::SPECULAR, modelPath);
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

    return std::make_unique<Mesh>(_window, vertices, indices, textures, STATIC_DRAW);
}

std::vector<std::shared_ptr<Texture>>
Model::loadMaterialTextures(aiMaterial *material,
                            aiTextureType assimpTextureType,
                            TextureType textureType,
                            std::filesystem::path const &modelPath)
{
    if (material == nullptr)
    {
        throw std::invalid_argument("Nullptr passed a pointer to the material.");
    }

    std::vector<std::shared_ptr<Texture>> textures;
    for (types::UInt i = 0; i < material->GetTextureCount(assimpTextureType); i++)
    {
        aiString fileName;
        material->GetTexture(assimpTextureType, i, &fileName);

        std::filesystem::path textureFilePath = modelPath.parent_path().append(fileName.C_Str());

        auto textureIsAlreadyLoaded =
            std::find_if(_loadedTextures.begin(),
                         _loadedTextures.end(),
                         [textureFilePath](std::shared_ptr<Texture> const &loadedTexture)
                         { return loadedTexture->filePath() == textureFilePath; });

        if (textureIsAlreadyLoaded == _loadedTextures.end())
        {
            std::shared_ptr<Texture> texture =
                std::make_shared<Texture>(_window, textureFilePath.string(), textureType);
            textures.push_back(texture);
        }
    }
    return textures;
}

} // namespace pf::gl
