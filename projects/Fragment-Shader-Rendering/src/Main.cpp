#include <array>
#include <iterator>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <iostream>
#include <fstream>
#include <ostream>
#include <chrono>
#include <cmath>
#include <filesystem>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <pf_gl/GLFW.hpp>
#include <pf_gl/Window.hpp>
#include <pf_gl/VertexLayout.hpp>
#include <pf_gl/VertexArray.hpp>
#include <pf_gl/Shader.hpp>
#include <pf_gl/Mesh.hpp>
#include <pf_gl/Texture.hpp>
#include <pf_gl/RenderingOptions.hpp>
#include <pf_utils/VideoEncoder.hpp>
#include <pf_gl/DrawingContext3D.hpp>

std::filesystem::path const
    TEXTURE_PATH("projects/Fragment-Shader-Rendering/res/textures/texture.bmp");

std::filesystem::path const
    FRAGMENT_SHADER_PATH("projects/Fragment-Shader-Rendering/res/shaders/fractal.fs");

std::filesystem::path const
    VERTEX_SHADER_PATH("projects/Fragment-Shader-Rendering/res/shaders/simple.vs");

std::filesystem::path const OUTPUT_FILE_PATH("projects/Fragment-Shader-Rendering/out/video.webm");

pf::gl::types::Size const WINDOW_WIDTH = 64, WINDOW_HEIGHT = 64;
pf::gl::types::Size const FRAME_BUFFER_WIDTH = 1080, FRAME_BUFFER_HEIGHT = 1080;
pf::gl::types::Size const FPS = 60;
pf::gl::types::Size const CRF = 30;
// (In seconds.)
pf::gl::types::Float const LOOP_DURATION = 30.0F;


int main(int /*argc*/, const char ** /*argv*/)
{
    // * Window setup *

    auto api = std::make_shared<pf::gl::GLFW>();
    auto window = std::make_shared<pf::gl::Window>(api, WINDOW_WIDTH, WINDOW_HEIGHT, "", 4, 6);
    window->initialize();
    window->bindContext();


    // * Create rectangle mesh *

    std::array<GLfloat, 12> rectangleVertices = {
        -1.0F,
        -1.0F,
        0.0F,

        1.0F,
        -1.0F,
        0.0F,

        -1.0F,
        1.0F,
        0.0F,

        1.0F,
        1.0F,
        0.0F,
    };
    std::vector<GLuint> rectangleIndices = {0, 1, 2, 1, 2, 3};

    std::shared_ptr<pf::gl::Texture> diffuseTexture =
        std::make_shared<pf::gl::Texture>(window, TEXTURE_PATH, pf::gl::TextureType::DIFFUSE);

    pf::gl::Mesh rectangleMesh(
        window,
        pf::util::RawBuffer(rectangleVertices),
        pf::gl::VertexLayout({{pf::gl::types::FLOAT_VECTOR_3, pf::gl::POSITION, GL_FALSE}}),
        rectangleIndices,
        {diffuseTexture},
        pf::gl::STATIC_DRAW);

    pf::gl::types::UInt frameBuffer = 0;
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    pf::gl::DrawingContext3D drawingContext{
        .viewportSize = glm::ivec2(FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT),
    };


    // * Image texture setup *

    pf::gl::types::UInt imageTexture = 0;
    glGenTextures(1, &imageTexture);
    glBindTexture(GL_TEXTURE_2D, imageTexture);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 FRAME_BUFFER_WIDTH,
                 FRAME_BUFFER_HEIGHT,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
    glBindTexture(GL_TEXTURE_2D, 0);


    // * Framebuffer setup *

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, imageTexture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        glDeleteFramebuffers(1, &frameBuffer);
        std::cerr << "Framebuffer is not complete";
        return 1;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // * Shader, video encoder, loop variables *

    std::vector<uint8_t> buffer(FRAME_BUFFER_WIDTH * FRAME_BUFFER_HEIGHT * 3, 0);
    pf::gl::types::Size frameIndex = 0;

    auto lastUpdateTime = std::chrono::high_resolution_clock::now();
    auto const appStartTime = lastUpdateTime;

    auto videoEncoder = pf::util::VideoEncoder::crf(
        OUTPUT_FILE_PATH, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, FPS, CRF);
    videoEncoder->start();

    pf::gl::Shader shader(window, VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH);
    shader.setUniformValue("u_loopDuration", LOOP_DURATION);


    // * Main loop *

    while (window->isOpen())
    {
        auto currentTime = std::chrono::high_resolution_clock::now();

        pf::gl::types::Float secondsSinceStart =
            std::chrono::duration<float>(currentTime - appStartTime).count();

        drawingContext.elapsedTimeSeconds = static_cast<pf::gl::types::Float>(frameIndex) / FPS;

        pf::gl::types::Float deltaSeconds =
            std::chrono::duration<float>(currentTime - lastUpdateTime).count();
        lastUpdateTime = currentTime;

        api->pollEvents();

        std::cout << "Rendering frame " << frameIndex << "/"
                  << static_cast<pf::gl::types::UInt>(LOOP_DURATION * FPS) << "..." << std::endl;

        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
        glClearColor(1.0F, 0.0F, 0.0F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);

        rectangleMesh.render(shader, drawingContext);

        glReadPixels(
            0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
        videoEncoder->appendFrameFromRGB(buffer);

        frameIndex++;
        if (frameIndex == static_cast<pf::gl::types::Size>(LOOP_DURATION * FPS))
        {
            break;
        }

        window->swapBuffers();
    }


    // * Clean up *

    glDeleteFramebuffers(1, &frameBuffer);
    videoEncoder->finish();

    return 0;
}
