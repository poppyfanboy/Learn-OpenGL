#ifndef GLFW_HPP
#define GLFW_HPP

#include <string>
#include <memory>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <pf_gl/ValueTypes.hpp>

namespace pf::gl
{

// TODO(poppyfanboy) Add a way to connect a logger.

/**
 * Encapsulates both a window and a context. A magic value which is passed around but it is not
 * accessed directly.
 */
using GLContext = GLFWwindow *;

/**
 * Directly handles events from the OS.
 */
class WindowCallbackHandler
{
public:
    virtual ~WindowCallbackHandler() = default;

    virtual void
    handleKeyEvent(types::UInt key, types::UInt scanCode, types::UInt action, types::UInt mods) = 0;
    virtual void handleResizeEvent(types::Size newWidth, types::Size newHeight) = 0;
    virtual void handleCursorMoveEvent(types::DVec2 newCursorPosition) = 0;
};

/**
 * An adapter class for the GLFW library.
 */
class GLFW final
{
public:
    // NOLINTNEXTLINE(misc-misplaced-const)
    static GLContext constexpr NULL_CONTEXT = nullptr;

    static types::UInt const DEFAULT_GL_CONTEXT_MAJOR_VERSION;
    static types::UInt const DEFAULT_GL_CONTEXT_MINOR_VERSION;
    static std::string const DEFAULT_WINDOW_TITLE;

    GLFW();

    GLFW(GLFW const &) = delete;
    GLFW(GLFW &&) = default;

    ~GLFW();

    GLFW &operator=(GLFW const &) = delete;
    GLFW &operator=(GLFW &&) = default;

    GLContext createWindow(std::shared_ptr<WindowCallbackHandler> const &callbackObject,
                           types::Size width,
                           types::Size height,
                           std::string const &title = DEFAULT_WINDOW_TITLE,
                           types::UInt contextMajorVersion = DEFAULT_GL_CONTEXT_MAJOR_VERSION,
                           types::UInt contextMinorVersion = DEFAULT_GL_CONTEXT_MINOR_VERSION);

    GLContext createWindow(types::Size width,
                           types::Size height,
                           std::string const &title = DEFAULT_WINDOW_TITLE,
                           types::UInt contextMajorVersion = DEFAULT_GL_CONTEXT_MAJOR_VERSION,
                           types::UInt contextMinorVersion = DEFAULT_GL_CONTEXT_MINOR_VERSION);

    // * Context and window lifetime *

    void bindContext(GLContext context);
    void destroyContext(GLContext context);
    bool isWindowOpen(GLContext context);
    void closeWindow(GLContext context);

    // * Events and inputs stuff *
    void pollEvents();
    bool isKeyPressed(GLContext context, types::UInt keyCode);

    // * Other *

    void swapBuffers(GLContext context);
    void hideCursor(GLContext context, bool hide);

private:
    GLContext _boundContext = NULL_CONTEXT;
};

} // namespace pf::gl

#endif // !GLFW_HPP
