#include <pf_gl/Window.hpp>

#include <cmath>
#include <stdexcept>
#include <memory>
#include <utility>
#include <vector>

#include <gsl/util>

#include <pf_gl/ValueTypes.hpp>
#include <pf_gl/GLFW.hpp>

namespace pf::gl
{

Window::Window(std::shared_ptr<GLFW> api,
               types::Size width,
               types::Size height,
               std::string title,
               types::UInt contextMajorVersion,
               types::UInt contextMinorVersion)
    : _api(std::move(api))
    , _width(width)
    , _height(height)
    , _title(std::move(title))
    , _contextMajorVersion(contextMajorVersion)
    , _contextMinorVersion(contextMinorVersion)
    , _context(nullptr)
    , _eventManager(std::make_shared<EventManager>(*this))
{
}

void Window::initialize()
{
    if (_initialized)
    {
        throw std::runtime_error("The window has already been initialized.");
    }

    _context = _api->createWindow(
        _eventManager, _width, _height, _title, _contextMajorVersion, _contextMinorVersion);
    _api->bindContext(_context);
    glViewport(0, 0, _width, _height);
    _initialized = true;
}

void Window::bindContext()
{
    lazyInitialize();
    if (!isOpen())
    {
        return;
    }

    _api->bindContext(_context);
}

void Window::swapBuffers()
{
    lazyInitialize();
    if (!isOpen())
    {
        return;
    }

    _api->swapBuffers(_context);
}

bool Window::isOpen() const
{
    if (!_initialized)
    {
        return false;
    }

    return _api->isWindowOpen(_context);
}

bool Window::isKeyPressed(types::UInt keyCode) const
{
    if (!isOpen())
    {
        return false;
    }

    return _api->isKeyPressed(_context, keyCode);
}

types::Size Window::width() const
{
    return _width;
}

types::Size Window::height() const
{
    return _height;
}

types::Float Window::aspectRatio() const
{
    return gsl::narrow_cast<pf::gl::types::Float>(width()) /
           gsl::narrow_cast<pf::gl::types::Float>(height());
}

types::DVec2 Window::mousePosition() const
{
    return _mousePosition;
}

void Window::close()
{
    if (!isOpen())
    {
        return;
    }

    _api->closeWindow(_context);
}

void Window::enableCursor(bool value)
{
    if (!isOpen())
    {
        return;
    }

    _api->hideCursor(_context, !value);
}

EventManager &Window::events() const
{
    return *_eventManager;
}

Window::~Window()
{
    if (_initialized)
    {
        _api->destroyContext(_context);
    }
}

void Window::lazyInitialize()
{
    if (!_initialized)
    {
        initialize();
    }
}

void Window::setKeyPressData(types::UInt key,
                             types::UInt scanCode,
                             types::UInt action,
                             types::UInt mods)
{
    // Alt + Enter for the fullscreen?
    // Is it OK to add a certain behavior for every window? A user won't be able to override this
    // stuff, I guess, that's a bad thing.
}

void Window::setMouseMoveData(types::DVec2 const &newPosition)
{
    _mousePosition = newPosition;
}

void Window::setResizeData(types::Size newWidth, types::Size newHeight)
{
    bindContext();
    glViewport(0, 0, newWidth, newHeight);
    _width = newWidth;
    _height = newHeight;
}

void EventManager::handleKeyEvent(types::UInt key,
                                  types::UInt scanCode,
                                  types::UInt action,
                                  types::UInt mods)
{
    _ownerWindow.setKeyPressData(key, scanCode, action, mods);
    notify(Window::EventType::KEY_PRESSED);
}

void EventManager::handleResizeEvent(types::Size newWidth, types::Size newHeight)
{
    _ownerWindow.setResizeData(newWidth, newHeight);
    notify(Window::EventType::RESIZED);
}

void EventManager::handleCursorMoveEvent(types::DVec2 newCursorPosition)
{
    _ownerWindow.setMouseMoveData(newCursorPosition);
    notify(Window::EventType::CURSOR_MOVED);
}

void EventManager::subscribe(Window::EventType eventType,
                             std::shared_ptr<Window::EventListener> const &listener)
{
    if (_listeners.find(eventType) == _listeners.end())
    {
        _listeners.at(eventType) = std::vector<std::weak_ptr<Window::EventListener>>();
    }
    _listeners.at(eventType).push_back(listener);
}

void EventManager::unsubscribe(Window::EventType eventType,
                               std::shared_ptr<Window::EventListener> const &listener)
{
    if (_listeners.find(eventType) == _listeners.end())
    {
        return;
    }
    auto &eventTypeListeners = _listeners.at(eventType);

    eventTypeListeners.erase(
        std::remove_if(eventTypeListeners.begin(),
                       eventTypeListeners.end(),
                       [listener](std::weak_ptr<Window::EventListener> const &storedListener)
                       { return storedListener.expired() || storedListener.lock() == listener; }),
        eventTypeListeners.end());
}

EventManager::EventManager(Window &ownerWindow)
    : _ownerWindow(ownerWindow)
{
}

void EventManager::notify(Window::EventType eventType)
{
    if (_listeners.find(eventType) == _listeners.end())
    {
        return;
    }
    auto &eventTypeListeners = _listeners.at(eventType);
    for (auto const &listener : eventTypeListeners)
    {
        listener.lock()->windowUpdate(_ownerWindow);
    }
}

} // namespace pf::gl
