#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <string>
#include <cmath>
#include <memory>
#include <cstddef>
#include <vector>

#include <sparsepp/spp.h>

#include <pf_gl/GLFW.hpp>
#include <pf_gl/ValueTypes.hpp>

namespace pf::gl
{

// (Forward declaration.)
class EventManager;

/**
 * A lazily initialized window class which also encapsulates an OpenGL context object. Window within
 * the DE is only created after the `initialize` method is called or any other method which requires
 * the window to be drawn on screen.
 *
 * Subscribe for events through the object returned from `events` method. Event listeners are
 * expected to poll the data they need from the Window class.
 */
class Window
{
public:
    enum EventType
    {
        KEY_PRESSED,
        CURSOR_MOVED,
        RESIZED,
    };

    class EventListener
    {
    public:
        virtual void windowUpdate(Window const &window) = 0;
    };

    Window(std::shared_ptr<GLFW> api,
           types::Size width,
           types::Size height,
           std::string title = "",
           types::UInt contextMajorVersion = 3,
           types::UInt contextMinorVersion = 3);

    Window(Window const &) = delete;
    Window(Window &&) = default;

    virtual ~Window();

    Window &operator=(Window const &) = delete;
    Window &operator=(Window &&) = default;

    [[nodiscard]] types::Size width() const;
    [[nodiscard]] types::Size height() const;
    [[nodiscard]] types::Float aspectRatio() const;
    [[nodiscard]] types::DVec2 mousePosition() const;

    /**
     * Returns `true` in case the window has been initialized and it is not going to close / it is
     * not already closed.
     */
    [[nodiscard]] bool isOpen() const;

    /**
     * Polls the data.
     */
    [[nodiscard]] bool isKeyPressed(types::UInt key) const;

    /**
     * Use the returned object to subscribe or unsubscribe for events.
     */
    [[nodiscard]] EventManager &events() const;

    /**
     * Creates the window within the DE.
     */
    void initialize();

    void bindContext();
    void swapBuffers();
    void enableCursor(bool value);
    void close();

private:
    friend class EventManager;

    std::shared_ptr<GLFW> _api;

    /**
     * True in case `initialize` method has been called. Keep in mind that the window can be closed
     * even if the value is true.
     */
    GLContext _context;
    std::shared_ptr<EventManager> _eventManager;

    bool _initialized = false;
    types::Size _width, _height;
    types::DVec2 _mousePosition = types::DVec2(std::nan(""));

    // Stored just because of lazy initialization
    std::string _title;
    types::UInt _contextMajorVersion, _contextMinorVersion;

    void lazyInitialize();
    void setMouseMoveData(types::DVec2 const &newPosition);
    void setResizeData(types::Size newWidth, types::Size newHeight);

    void
    setKeyPressData(types::UInt key, types::UInt scanCode, types::UInt action, types::UInt mods);
};

/**
 * This class receives the events from GLFW and processes them: first it updates the state of the
 * window and then it notifies the listeners. It also handles listeners (un)subscribptions.
 */
class EventManager : public WindowCallbackHandler
{
public:
    explicit EventManager(Window &ownerWindow);

    void subscribe(Window::EventType eventType,
                   std::shared_ptr<Window::EventListener> const &listener);

    void unsubscribe(Window::EventType eventType,
                     std::shared_ptr<Window::EventListener> const &listener);

private:
    friend class Window;

    Window &_ownerWindow;

    spp::sparse_hash_map<Window::EventType, std::vector<std::weak_ptr<Window::EventListener>>>
        _listeners;

    // HACK(poppyfanboy) public virtual methods are overriden as private, so that the public
    // interface of the `EventManager` is not cluttered. Not sure if it's ok to leave it like that.
    void handleCursorMoveEvent(types::DVec2 newCursorPosition) override;
    void handleResizeEvent(types::Size newWidth, types::Size newHeight) override;
    void handleKeyEvent(types::UInt key,
                        types::UInt scanCode,
                        types::UInt action,
                        types::UInt mods) override;

    void notify(Window::EventType eventType);
};

} // namespace pf::gl

#endif // !WINDOW_HPP
