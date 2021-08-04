#include <array>
#include <cmath>
#include <iterator>
#include <memory>
#include <string>
#include <chrono>
#include <iostream>
#include <filesystem>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <gsl/util>

#include <pf_gl/GLFW.hpp>
#include <pf_gl/Window.hpp>
#include <pf_gl/VertexBuffer.hpp>
#include <pf_gl/ElementBuffer.hpp>
#include <pf_gl/VertexArray.hpp>
#include <pf_gl/Shader.hpp>
#include <pf_gl/MinecraftCamera.hpp>
#include <pf_gl/Texture.hpp>
#include <pf_gl/EulerTransform3D.hpp>
#include <pf_gl/Transform3D.hpp>
#include <pf_gl/Mesh.hpp>
#include <pf_gl/Model.hpp>
#include <pf_gl/RenderingOptions.hpp>
#include <pf_gl/ValueTypes.hpp>
#include <pf_utils/IndexedString.hpp>

pf::gl::types::Size const WINDOW_WIDTH = 1600;
pf::gl::types::Size const WINDOW_HEIGHT = 900;

std::filesystem::path const CUBE_LIGHT_VERTEX_SHADER_PATH =
    "projects/Learn-OpenGL/res/shaders/cube-light.vs";

std::filesystem::path const CUBE_LIGHT_FRAGMENT_SHADER_PATH =
    "projects/Learn-OpenGL/res/shaders/cube-light.fs";

std::filesystem::path const COLOR_VERTEX_SHADER_PATH =
    "projects/Learn-OpenGL/res/shaders/lighting.vs";

std::filesystem::path const COLOR_FRAGMENT_SHADER_PATH =
    "projects/Learn-OpenGL/res/shaders/lighting.fs";

std::filesystem::path const CONTAINER_TEXTURE_PATH =
    "projects/Learn-OpenGL/res/textures/container.png";

std::filesystem::path const CONTAINER_SPECULAR_MAP_PATH =
    "projects/Learn-OpenGL/res/textures/container-specular-map.png";

std::filesystem::path const MODEL_PATH = "projects/Learn-OpenGL/res/models/barrel.obj";


int main(int /*argc*/, const char ** /*argv*/)
{
    auto api = std::make_shared<pf::gl::GLFW>();
    auto window =
        std::make_shared<pf::gl::Window>(api, WINDOW_WIDTH, WINDOW_HEIGHT, "Lighting", 4, 6);
    window->initialize();
    window->bindContext();
    window->enableCursor(false);

    pf::gl::Shader cubeLightShader(
        window, CUBE_LIGHT_VERTEX_SHADER_PATH, CUBE_LIGHT_FRAGMENT_SHADER_PATH);
    pf::gl::Shader lightingShader(window, COLOR_VERTEX_SHADER_PATH, COLOR_FRAGMENT_SHADER_PATH);

    pf::gl::MinecraftCamera camera = pf::gl::MinecraftCamera::Builder()
                                         .withAspectRatio(window->aspectRatio())
                                         .atPosition(glm::vec3(0.0F, 0.0F, 3.0F))
                                         .withFOV(std::numbers::pi / 3.0F)
                                         .build();

    glEnable(GL_DEPTH_TEST);

    std::array<GLfloat, 288> cubeRawVertices = {
        -0.5F, -0.5F, -0.5F, 0.0F,  0.0F,  -1.0F, 0.0F, 0.0F,

        0.5F,  -0.5F, -0.5F, 0.0F,  0.0F,  -1.0F, 1.0F, 0.0F,

        0.5F,  0.5F,  -0.5F, 0.0F,  0.0F,  -1.0F, 1.0F, 1.0F,

        0.5F,  0.5F,  -0.5F, 0.0F,  0.0F,  -1.0F, 1.0F, 1.0F,

        -0.5F, 0.5F,  -0.5F, 0.0F,  0.0F,  -1.0F, 0.0F, 1.0F,

        -0.5F, -0.5F, -0.5F, 0.0F,  0.0F,  -1.0F, 0.0F, 0.0F,


        -0.5F, -0.5F, 0.5F,  0.0F,  0.0F,  1.0F,  0.0F, 0.0F,

        0.5F,  -0.5F, 0.5F,  0.0F,  0.0F,  1.0F,  1.0F, 0.0F,

        0.5F,  0.5F,  0.5F,  0.0F,  0.0F,  1.0F,  1.0F, 1.0F,

        0.5F,  0.5F,  0.5F,  0.0F,  0.0F,  1.0F,  1.0F, 1.0F,

        -0.5F, 0.5F,  0.5F,  0.0F,  0.0F,  1.0F,  0.0F, 1.0F,

        -0.5F, -0.5F, 0.5F,  0.0F,  0.0F,  1.0F,  0.0F, 0.0F,


        -0.5F, 0.5F,  0.5F,  -1.0F, 0.0F,  0.0F,  1.0F, 0.0F,

        -0.5F, 0.5F,  -0.5F, -1.0F, 0.0F,  0.0F,  1.0F, 1.0F,

        -0.5F, -0.5F, -0.5F, -1.0F, 0.0F,  0.0F,  0.0F, 1.0F,

        -0.5F, -0.5F, -0.5F, -1.0F, 0.0F,  0.0F,  0.0F, 1.0F,

        -0.5F, -0.5F, 0.5F,  -1.0F, 0.0F,  0.0F,  0.0F, 0.0F,

        -0.5F, 0.5F,  0.5F,  -1.0F, 0.0F,  0.0F,  1.0F, 0.0F,


        0.5F,  0.5F,  0.5F,  1.0F,  0.0F,  0.0F,  1.0F, 0.0F,

        0.5F,  0.5F,  -0.5F, 1.0F,  0.0F,  0.0F,  1.0F, 1.0F,

        0.5F,  -0.5F, -0.5F, 1.0F,  0.0F,  0.0F,  0.0F, 1.0F,

        0.5F,  -0.5F, -0.5F, 1.0F,  0.0F,  0.0F,  0.0F, 1.0F,

        0.5F,  -0.5F, 0.5F,  1.0F,  0.0F,  0.0F,  0.0F, 0.0F,

        0.5F,  0.5F,  0.5F,  1.0F,  0.0F,  0.0F,  1.0F, 0.0F,


        -0.5F, -0.5F, -0.5F, 0.0F,  -1.0F, 0.0F,  0.0F, 1.0F,

        0.5F,  -0.5F, -0.5F, 0.0F,  -1.0F, 0.0F,  1.0F, 1.0F,

        0.5F,  -0.5F, 0.5F,  0.0F,  -1.0F, 0.0F,  1.0F, 0.0F,

        0.5F,  -0.5F, 0.5F,  0.0F,  -1.0F, 0.0F,  1.0F, 0.0F,

        -0.5F, -0.5F, 0.5F,  0.0F,  -1.0F, 0.0F,  0.0F, 0.0F,

        -0.5F, -0.5F, -0.5F, 0.0F,  -1.0F, 0.0F,  0.0F, 1.0F,


        -0.5F, 0.5F,  -0.5F, 0.0F,  1.0F,  0.0F,  0.0F, 1.0F,

        0.5F,  0.5F,  -0.5F, 0.0F,  1.0F,  0.0F,  1.0F, 1.0F,

        0.5F,  0.5F,  0.5F,  0.0F,  1.0F,  0.0F,  1.0F, 0.0F,

        0.5F,  0.5F,  0.5F,  0.0F,  1.0F,  0.0F,  1.0F, 0.0F,

        -0.5F, 0.5F,  0.5F,  0.0F,  1.0F,  0.0F,  0.0F, 0.0F,

        -0.5F, 0.5F,  -0.5F, 0.0F,  1.0F,  0.0F,  0.0F, 1.0F,
    };

    std::vector<pf::gl::Mesh::SimpleVertex> cubeVertices;
    for (size_t i = 0; i < 36; i++)
    {
        cubeVertices.push_back(
            {glm::vec3(cubeRawVertices.at(8 * i),
                       cubeRawVertices.at(8 * i + 1),
                       cubeRawVertices.at(8 * i + 2)),
             glm::vec3(cubeRawVertices.at(8 * i + 3),
                       cubeRawVertices.at(8 * i + 4),
                       cubeRawVertices.at(8 * i + 5)),
             glm::vec2(cubeRawVertices.at(8 * i + 6), cubeRawVertices.at(8 * i + 7))});
    }

    std::vector<pf::gl::types::UInt> cubeIndices;
    for (pf::gl::types::UInt i = 0; i < 36; i++)
    {
        cubeIndices.push_back(i);
    }

    std::array<glm::vec3, 10> barrelsPositions = {glm::vec3(0.0F, 0.0F, 0.0F),
                                                  glm::vec3(2.0F, 5.0F, -15.0F),
                                                  glm::vec3(-1.5F, -2.2F, -2.5F),
                                                  glm::vec3(-3.8F, -2.0F, -12.3F),
                                                  glm::vec3(2.4F, -0.4F, -3.5F),
                                                  glm::vec3(-1.7F, 3.0F, -7.5F),
                                                  glm::vec3(1.3F, -2.0F, -2.5F),
                                                  glm::vec3(1.5F, 2.0F, -2.5F),
                                                  glm::vec3(1.5F, 0.2F, -1.5F),
                                                  glm::vec3(-1.3F, 1.0F, -1.5F)};

    std::array<float, 10> barrelsRotationSpeeds = {
        45.0F,
        -30.0F,
        90.0F,
        -15.0F,
        30.0F,
        60.0F,
        -10.0F,
        30.0F,
        70.0F,
        -40.0F,
    };

    std::array<glm::vec3, 4> pointLightsPositions = {
        glm::vec3(0.7F, 0.2F, 2.0F),
        glm::vec3(2.3F, -3.3F, -4.0F),
        glm::vec3(-4.0F, 2.0F, -12.0F),
        glm::vec3(0.0F, 0.0F, -3.0F),
    };

    std::array<pf::util::IndexedString, 4> pointLightsUniformNames = {
        pf::util::IndexedString("u_pointLights[0].????????????????"),
        pf::util::IndexedString("u_pointLights[1].????????????????"),
        pf::util::IndexedString("u_pointLights[2].????????????????"),
        pf::util::IndexedString("u_pointLights[3].????????????????"),
    };


    std::vector<std::unique_ptr<pf::gl::Model>> barrelsModels;
    for (auto const &position : barrelsPositions)
    {
        std::unique_ptr<pf::gl::Transform3D> transform =
            pf::gl::EulerTransform3D::Builder().withShift(position).build();
        auto model = std::make_unique<pf::gl::Model>(window, MODEL_PATH, std::move(transform));
        barrelsModels.emplace_back(std::move(model));
    }

    std::vector<std::unique_ptr<pf::gl::Model>> pointLightsModels;
    for (auto const &position : pointLightsPositions)
    {
        std::unique_ptr<pf::gl::Transform3D> transform = pf::gl::EulerTransform3D::Builder()
                                                             .withShift(position)
                                                             .withScale(glm::vec3(0.2F, 0.2F, 0.2F))
                                                             .build();

        auto model = std::make_unique<pf::gl::Model>(
            window,
            std::vector<std::shared_ptr<pf::gl::Mesh>>({
                std::make_shared<pf::gl::Mesh>(window,
                                               cubeVertices,
                                               cubeIndices,
                                               std::vector<std::shared_ptr<pf::gl::Texture>>({}),
                                               pf::gl::STATIC_DRAW),
            }),
            std::move(transform));
        pointLightsModels.emplace_back(std::move(model));
    }

    std::array<glm::vec3, 4> pointLightsColors = {
        glm::vec3(1.0, 0.3, 0.1),
        glm::vec3(0.3, 1.0, 0.1),
        glm::vec3(1.0, 0.6, 1.0),
        glm::vec3(0.2, 1.0, 0.9),
    };

    auto lastUpdateTime = std::chrono::high_resolution_clock::now();
    auto const appStartTime = lastUpdateTime;

    glm::dvec2 lastMousePosition = window->mousePosition();
    while (window->isOpen())
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float secondsSinceStart = std::chrono::duration<float>(currentTime - appStartTime).count();

        float deltaSeconds = std::chrono::duration<float>(currentTime - lastUpdateTime).count();
        lastUpdateTime = currentTime;

        api->pollEvents();

        glm::vec3 inputVector =
            glm::vec3(static_cast<int>(window->isKeyPressed(GLFW_KEY_W)) -
                          static_cast<int>(window->isKeyPressed(GLFW_KEY_S)),
                      static_cast<int>(window->isKeyPressed(GLFW_KEY_D)) -
                          static_cast<int>(window->isKeyPressed(GLFW_KEY_A)),
                      static_cast<int>(window->isKeyPressed(GLFW_KEY_LEFT_SHIFT)) -
                          static_cast<int>(window->isKeyPressed(GLFW_KEY_SPACE)));

        glm::vec2 cameraDelta = window->mousePosition() - lastMousePosition;
        if (std::isnan(cameraDelta.x) || std::isnan(cameraDelta.y))
        {
            cameraDelta = glm::vec2();
        }
        lastMousePosition = window->mousePosition();

        camera.move(inputVector, deltaSeconds);
        camera.rotate(cameraDelta, deltaSeconds);
        camera.aspectRatio(gsl::narrow_cast<pf::gl::types::Float>(window->width()) /
                           gsl::narrow_cast<pf::gl::types::Float>(window->height()));

        glm::mat4 view = camera.viewMatrix();

        glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // Lighting setup

        lightingShader.use();
        lightingShader.setUniformValue("u_directionalLight.direction",
                                       glm::vec3(-0.2F, -1.0F, -0.3F));
        lightingShader.setUniformValue("u_directionalLight.ambient",
                                       glm::vec3(0.05F, 0.05F, 0.05F));
        lightingShader.setUniformValue("u_directionalLight.diffuse", glm::vec3(0.4F, 0.4F, 0.4F));
        lightingShader.setUniformValue("u_directionalLight.specular", glm::vec3(0.5F, 0.5F, 0.5F));

        for (size_t i = 0; i < pointLightsPositions.size(); i++)
        {
            lightingShader.setUniformValue(
                pointLightsUniformNames.at(i).withIndex("position"),
                glm::vec3(view * glm::vec4(pointLightsPositions.at(i), 1.0F)));
            lightingShader.setUniformValue(pointLightsUniformNames.at(i).withIndex("ambient"),
                                           glm::vec3(0.05F, 0.05F, 0.05F));
            lightingShader.setUniformValue(pointLightsUniformNames.at(i).withIndex("diffuse"),
                                           pointLightsColors.at(i));
            lightingShader.setUniformValue(pointLightsUniformNames.at(i).withIndex("specular"),
                                           glm::vec3(1.0F, 1.0F, 1.0F));
            lightingShader.setUniformValue(
                pointLightsUniformNames.at(i).withIndex("constantFactor"), 1.0F);
            lightingShader.setUniformValue(pointLightsUniformNames.at(i).withIndex("linearFactor"),
                                           0.09F);
            lightingShader.setUniformValue(
                pointLightsUniformNames.at(i).withIndex("quadraticFactor"), 0.032F);
        }

        lightingShader.setUniformValue("u_spotLight.position", glm::vec3(0.0F));
        lightingShader.setUniformValue("u_spotLight.direction", glm::vec3(0.0F, 0.0F, -1.0F));
        lightingShader.setUniformValue("u_spotLight.ambient", glm::vec3(0.0F, 0.0F, 0.0F));
        lightingShader.setUniformValue("u_spotLight.diffuse", glm::vec3(0.0F, 1.0F, 1.0F));
        lightingShader.setUniformValue("u_spotLight.specular", glm::vec3(1.0F, 1.0F, 1.0F));
        lightingShader.setUniformValue("u_spotLight.constantFactor", 1.0F);
        lightingShader.setUniformValue("u_spotLight.linearFactor", 0.09F);
        lightingShader.setUniformValue("u_spotLight.quadraticFactor", 0.032F);
        lightingShader.setUniformValue("u_spotLight.cosCutOff", glm::cos(glm::radians(12.0F)));
        lightingShader.setUniformValue("u_spotLight.cosOuterCutOff", glm::cos(glm::radians(15.0F)));

        // Drawing meshes

        auto *cubesRotationSpeedsIterator = barrelsRotationSpeeds.begin();
        for (auto const &model : barrelsModels)
        {
            lightingShader.use();
            lightingShader.setUniformValue("u_shininess", 32.0F);

            auto scale = pf::gl::EulerTransform3D::Builder()
                             .withScale(glm::vec3(1.0F + 0.0035F * std::sin(secondsSinceStart)))
                             .build();

            glm::vec3 up = glm::vec3(0.0F, 1.0F, 0.0) * 0.9F / 2.0F;

            auto translate = pf::gl::EulerTransform3D::Builder().withShift(-up).build();
            auto translateBack = pf::gl::EulerTransform3D::Builder().withShift(up).build();

            auto rotate = pf::gl::EulerTransform3D::Builder().build();
            rotate = rotate->rotateAround(deltaSeconds * glm::radians(*cubesRotationSpeedsIterator),
                                          glm::vec3(1.0F, 0.5F, 0.0F));

            std::unique_ptr<pf::gl::Transform3D> deltaTransform =
                std::make_unique<pf::gl::EulerTransform3D>();

            deltaTransform = deltaTransform->combine(
                *translateBack->combine(*rotate->combine(*scale->combine(*translate))));

            model->transform(std::move(model->transform().combine(*deltaTransform)));
            model->render(lightingShader, camera);
            std::advance(cubesRotationSpeedsIterator, 1);
        }

        cubeLightShader.use();
        auto *pointLightsColorsIterator = pointLightsColors.begin();
        for (auto const &model : pointLightsModels)
        {
            cubeLightShader.use();
            cubeLightShader.setUniformValue("u_color", *pointLightsColorsIterator);
            model->render(cubeLightShader, camera);
            std::advance(pointLightsColorsIterator, 1);
        }

        window->swapBuffers();
    }

    return 0;
}
