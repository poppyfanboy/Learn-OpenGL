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
#include <pf_gl/DrawingContext3D.hpp>
#include <pf_gl/Material.hpp>

pf::gl::types::Size const WINDOW_WIDTH = 1600;
pf::gl::types::Size const WINDOW_HEIGHT = 900;

std::filesystem::path const SIMPLE_VERTEX_SHADER_PATH =
    "projects/Learn-OpenGL/res/shaders/simple.vs";

std::filesystem::path const COLOR_FRAGMENT_SHADER_PATH =
    "projects/Learn-OpenGL/res/shaders/color.fs";

std::filesystem::path const DEFAULT_VERTEX_SHADER_PATH =
    "projects/Learn-OpenGL/res/shaders/default.vs";

std::filesystem::path const LIGHTING_FRAGMENT_SHADER_PATH =
    "projects/Learn-OpenGL/res/shaders/lighting.fs";

std::filesystem::path const BARREL_MODEL_PATH = "projects/Learn-OpenGL/res/models/barrel.obj";

pf::gl::DrawingContext3D createDrawingContext(pf::gl::Window const &window)
{
    pf::gl::DrawingContext3D drawingContext;

    drawingContext.camera = pf::gl::MinecraftCamera::Builder()
                                .withAspectRatio(window.aspectRatio())
                                .atPosition(glm::vec3(0.0F, 0.0F, 3.0F))
                                .withFOV(std::numbers::pi / 3.0F)
                                .build();

    drawingContext.directionalLight = {
        .direction = glm::vec3(-0.2F, -1.0F, -0.3F),
        .color =
            {
                .ambient = glm::vec3(0.05F, 0.05F, 0.05F),
                .diffuse = glm::vec3(0.4F, 0.4F, 0.4F),
                .specular = glm::vec3(0.5F, 0.5F, 0.5F),
            },
    };

    pf::gl::LightFalloff pointLightFalloff = {
        .constant = 1.0F,
        .linear = 0.09F,
        .quadratic = 0.032F,
    };

    drawingContext.pointLights = {
        {
            .position = glm::vec3(0.7F, 0.2F, 2.0F),
            .color =
                {
                    .ambient = glm::vec3(0.05F, 0.05F, 0.05F),
                    .diffuse = glm::vec3(1.0F, 0.3F, 0.1F),
                    .specular = glm::vec3(1.0F, 1.0F, 1.0F),
                },
            .falloff = pointLightFalloff,
        },
        {
            .position = glm::vec3(2.3F, -3.3F, -4.0F),
            .color =
                {
                    .ambient = glm::vec3(0.05F, 0.05F, 0.05F),
                    .diffuse = glm::vec3(0.3F, 1.0F, 0.1F),
                    .specular = glm::vec3(1.0F, 1.0F, 1.0F),
                },
            .falloff = pointLightFalloff,
        },
        {
            .position = glm::vec3(-4.0F, 2.0F, -12.0F),
            .color =
                {
                    .ambient = glm::vec3(0.05F, 0.05F, 0.05F),
                    .diffuse = glm::vec3(1.0F, 0.6F, 1.0F),
                    .specular = glm::vec3(1.0F, 1.0F, 1.0F),
                },
            .falloff = pointLightFalloff,
        },
        {
            .position = glm::vec3(0.0F, 0.0F, -3.0F),
            .color =
                {
                    .ambient = glm::vec3(0.05F, 0.05F, 0.05F),
                    .diffuse = glm::vec3(0.2F, 1.0F, 0.9F),
                    .specular = glm::vec3(1.0F, 1.0F, 1.0F),
                },
            .falloff = pointLightFalloff,
        },
    };

    drawingContext.spotLight = {
        .position = glm::vec3(0.0F),
        .direction = glm::vec3(0.0F, 0.0F, -1.0F),
        .color =
            {
                .ambient = glm::vec3(0.0F),
                .diffuse = glm::vec3(0.0F, 1.0F, 1.0F),
                .specular = glm::vec3(1.0F, 1.0F, 1.0F),
            },
        .falloff =
            {
                .constant = 1.0F,
                .linear = 0.09F,
                .quadratic = 0.032F,
            },
        .cutoff = glm::cos(glm::radians(12.0F)),
        .outerCutoff = glm::cos(glm::radians(15.0F)),
    };

    return drawingContext;
}

std::unique_ptr<pf::gl::Mesh> createCubeMesh(std::shared_ptr<pf::gl::Window> const &window)
{
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
        cubeVertices.push_back({
            glm::vec3(cubeRawVertices.at(8 * i),
                      cubeRawVertices.at(8 * i + 1),
                      cubeRawVertices.at(8 * i + 2)),

            glm::vec3(cubeRawVertices.at(8 * i + 3),
                      cubeRawVertices.at(8 * i + 4),
                      cubeRawVertices.at(8 * i + 5)),

            glm::vec2(cubeRawVertices.at(8 * i + 6), cubeRawVertices.at(8 * i + 7)),
        });
    }

    std::vector<pf::gl::types::UInt> cubeIndices;
    for (pf::gl::types::UInt i = 0; i < 36; i++)
    {
        cubeIndices.push_back(i);
    }

    auto mesh = std::make_unique<pf::gl::Mesh>(window,
                                               cubeVertices,
                                               cubeIndices,
                                               std::vector<std::shared_ptr<pf::gl::Texture>>({}),
                                               pf::gl::STATIC_DRAW);

    return mesh;
}

struct Barrel
{
    glm::vec3 position;
    float rotationSpeed;
    std::unique_ptr<pf::gl::Model> model;
};

std::array<Barrel, 10> barrels = {
    Barrel{
        .position = glm::vec3(0.0F, 0.0F, 0.0F),
        .rotationSpeed = 45.0F,
    },
    Barrel{
        .position = glm::vec3(2.0F, 5.0F, -15.0F),
        .rotationSpeed = -30.0F,
    },
    Barrel{
        .position = glm::vec3(-1.5F, -2.2F, -2.5F),
        .rotationSpeed = 90.0F,
    },
    Barrel{
        .position = glm::vec3(-3.8F, -2.0F, -12.3F),
        .rotationSpeed = -15.0F,
    },
    Barrel{
        .position = glm::vec3(2.4F, -0.4F, -3.5F),
        .rotationSpeed = 30.0F,
    },
    Barrel{
        .position = glm::vec3(-1.7F, 3.0F, -7.5F),
        .rotationSpeed = 60.0F,
    },
    Barrel{
        .position = glm::vec3(1.3F, -2.0F, -2.5F),
        .rotationSpeed = -10.0F,
    },
    Barrel{
        .position = glm::vec3(1.5F, 2.0F, -2.5F),
        .rotationSpeed = 30.0F,
    },
    Barrel{
        .position = glm::vec3(1.5F, 0.2F, -1.5F),
        .rotationSpeed = 70.0F,
    },
    Barrel{
        .position = glm::vec3(-1.3F, 1.0F, -1.5F),
        .rotationSpeed = -40.0F,
    },
};

int main(int /*argc*/, const char ** /*argv*/)
{
    auto api = std::make_shared<pf::gl::GLFW>();
    auto window =
        std::make_shared<pf::gl::Window>(api, WINDOW_WIDTH, WINDOW_HEIGHT, "Lighting", 4, 6);
    window->initialize();
    window->enableCursor(false);
    glEnable(GL_DEPTH_TEST);

    pf::gl::Shader colorShader(window, SIMPLE_VERTEX_SHADER_PATH, COLOR_FRAGMENT_SHADER_PATH);
    pf::gl::Shader lightingShader(
        window, DEFAULT_VERTEX_SHADER_PATH, LIGHTING_FRAGMENT_SHADER_PATH);

    pf::gl::DrawingContext3D drawingContext = createDrawingContext(*window);


    // * Barrels *

    for (auto &barrel : barrels)
    {
        std::unique_ptr<pf::gl::Transform3D> transform =
            pf::gl::EulerTransform3D::Builder().withShift(barrel.position).build();
        barrel.model = std::make_unique<pf::gl::Model>(
            window, BARREL_MODEL_PATH, std::move(transform), pf::gl::Material{.shininess = 32.0F});
    }


    // * Point lights *

    std::shared_ptr<pf::gl::Mesh> cubeMesh = createCubeMesh(window);
    std::vector<std::unique_ptr<pf::gl::Model>> pointLightsModels;
    for (auto const &pointLight : drawingContext.pointLights.value())
    {
        std::unique_ptr<pf::gl::Transform3D> transform = pf::gl::EulerTransform3D::Builder()
                                                             .withShift(pointLight.position)
                                                             .withScale(glm::vec3(0.2F, 0.2F, 0.2F))
                                                             .build();

        auto model =
            std::make_unique<pf::gl::Model>(window,
                                            std::vector<std::shared_ptr<pf::gl::Mesh>>({cubeMesh}),
                                            std::move(transform),
                                            pf::gl::Material{.color = pointLight.color.diffuse});
        pointLightsModels.push_back(std::move(model));
    }


    // * Main loop *

    auto lastUpdateTime = std::chrono::high_resolution_clock::now();
    auto const appStartTime = lastUpdateTime;
    glm::dvec2 lastMousePosition = window->mousePosition();

    while (window->isOpen())
    {
        // * Update *

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

        drawingContext.camera->move(inputVector, deltaSeconds);
        drawingContext.camera->rotate(cameraDelta, deltaSeconds);
        drawingContext.camera->aspectRatio(
            gsl::narrow_cast<pf::gl::types::Float>(window->width()) /
            gsl::narrow_cast<pf::gl::types::Float>(window->height()));


        // * Draw *

        glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // barrels

        lightingShader.use();

        for (auto const &barrel : barrels)
        {
            auto scale = pf::gl::EulerTransform3D::Builder()
                             .withScale(glm::vec3(1.0F + 0.0035F * std::sin(secondsSinceStart)))
                             .build();

            glm::vec3 up = glm::vec3(0.0F, 1.0F, 0.0) * 0.9F / 2.0F;
            auto translate = pf::gl::EulerTransform3D::Builder().withShift(-up).build();
            auto translateBack = pf::gl::EulerTransform3D::Builder().withShift(up).build();
            auto rotate = pf::gl::EulerTransform3D::Builder().build()->rotateAround(
                deltaSeconds * glm::radians(barrel.rotationSpeed), glm::vec3(1.0F, 0.5F, 0.0F));

            std::unique_ptr<pf::gl::Transform3D> deltaTransform =
                std::make_unique<pf::gl::EulerTransform3D>();
            deltaTransform = deltaTransform->combine(
                *translateBack->combine(*rotate->combine(*scale->combine(*translate))));
            barrel.model->transform(std::move(barrel.model->transform().combine(*deltaTransform)));

            barrel.model->render(lightingShader, drawingContext);
        }

        // point lights

        colorShader.use();

        auto pointLightsIterator = drawingContext.pointLights->begin();
        auto pointLightsModelsIterator = pointLightsModels.begin();

        for (; pointLightsIterator != drawingContext.pointLights->end() &&
               pointLightsModelsIterator != pointLightsModels.end();
             pointLightsIterator++, pointLightsModelsIterator++)
        {
            (*pointLightsModelsIterator)->render(colorShader, drawingContext);
        }

        window->swapBuffers();
    }

    return 0;
}
