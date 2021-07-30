#include <pf_gl/GLFW.hpp>

#include <iostream>
#include <string>
#include <stdexcept>

#include <gsl/util>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <pf_gl/ValueTypes.hpp>

namespace pf::gl
{

types::UInt const GLFW::DEFAULT_GL_CONTEXT_MAJOR_VERSION = 3;
types::UInt const GLFW::DEFAULT_GL_CONTEXT_MINOR_VERSION = 3;
std::string const GLFW::DEFAULT_WINDOW_TITLE = "Untitled";

GLFW::GLFW()
{
    int initSuccessful = glfwInit();
    if (initSuccessful != GLFW_TRUE)
    {
        throw std::runtime_error("Failed to initialize GLFW.");
    }
    glfwSetErrorCallback(
        [](int error, const char *description) {
            std::cerr << "GLFW error (code: " << std::hex << error << "): " << description << "\n";
        });
}

GLFW::~GLFW()
{
    glfwTerminate();
}

GLContext GLFW::createWindow(std::shared_ptr<WindowCallbackHandler> const &callbackObject,
                             types::Size width,
                             types::Size height,
                             std::string const &title,
                             types::UInt contextMajorVersion,
                             types::UInt contextMinorVersion)
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gsl::narrow_cast<types::Int>(contextMajorVersion));
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gsl::narrow_cast<types::Int>(contextMinorVersion));
    GLContext context = glfwCreateWindow(gsl::narrow_cast<types::Int>(width),
                                         gsl::narrow_cast<types::Int>(height),
                                         title.c_str(),
                                         nullptr,
                                         nullptr);
    if (context == nullptr)
    {
        throw std::runtime_error("Failed to create a GLFW window.");
    }

    glfwMakeContextCurrent(context);
    _boundContext = context;

    if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)) == GL_FALSE)
    {
        throw std::runtime_error("Failed to load OpenGL functions.");
    }

    glfwSwapInterval(1);
    glfwSetWindowUserPointer(context, callbackObject.get());

    // * Callbacks setup *

    if (callbackObject != nullptr)
    {
        glfwSetKeyCallback(context,
                           [](GLFWwindow *glfwWindow, int key, int scanCode, int action, int mods)
                           {
                               auto *handler = reinterpret_cast<WindowCallbackHandler *>(
                                   glfwGetWindowUserPointer(glfwWindow));
                               if (handler != nullptr)
                               {
                                   handler->handleKeyEvent(key, scanCode, action, mods);
                               }
                           });
        glfwSetWindowSizeCallback(context,
                                  [](GLFWwindow *glfwWindow, int width, int height)
                                  {
                                      auto *handler = reinterpret_cast<WindowCallbackHandler *>(
                                          glfwGetWindowUserPointer(glfwWindow));
                                      if (handler != nullptr)
                                      {
                                          handler->handleResizeEvent(width, height);
                                      }
                                  });
        glfwSetCursorPosCallback(
            context,
            [](GLFWwindow *glfwWindow, double positionX, double positionY)
            {
                auto *handler =
                    reinterpret_cast<WindowCallbackHandler *>(glfwGetWindowUserPointer(glfwWindow));
                if (handler != nullptr)
                {
                    handler->handleCursorMoveEvent(types::DVec2(positionX, positionY));
                }
            });
    }

    return context;
}

GLContext GLFW::createWindow(types::Size width,
                             types::Size height,
                             std::string const &title,
                             types::UInt contextMajorVersion,
                             types::UInt contextMinorVersion)
{
    return createWindow(nullptr, width, height, title, contextMajorVersion, contextMinorVersion);
}

void GLFW::bindContext(GLContext context)
{
    if (context == _boundContext)
    {
        return;
    }

    glfwMakeContextCurrent(context);
    _boundContext = context;
}

void GLFW::swapBuffers(GLContext context)
{
    glfwSwapBuffers(context);
}

bool GLFW::isWindowOpen(GLContext context)
{
    return glfwWindowShouldClose(context) == GLFW_FALSE;
}

void GLFW::closeWindow(GLContext context)
{
    glfwSetWindowShouldClose(context, GLFW_TRUE);
}

bool GLFW::isKeyPressed(GLContext context, types::UInt keyCode)
{
    return glfwGetKey(context, gsl::narrow_cast<types::Int>(keyCode)) == 1;
}

void GLFW::hideCursor(GLContext context, bool hide)
{
    glfwSetInputMode(context, GLFW_CURSOR, hide ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void GLFW::pollEvents()
{
    glfwPollEvents();
}

void GLFW::destroyContext(GLContext context)
{
    glfwDestroyWindow(context);
    _boundContext = NULL_CONTEXT;
}

} // namespace pf::gl
