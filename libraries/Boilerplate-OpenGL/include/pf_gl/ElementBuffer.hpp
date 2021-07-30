#ifndef ELEMENT_BUFFER_HPP
#define ELEMENT_BUFFER_HPP

#include <memory>
#include <span>

#include <pf_gl/Window.hpp>
#include <pf_gl/RenderingOptions.hpp>
#include <pf_gl/ValueTypes.hpp>

namespace pf::gl
{

class ElementBuffer final
{
public:
    ElementBuffer(std::shared_ptr<Window> window,
                  std::span<const types::UInt> const &indices,
                  UsagePattern usagePattern);

    ElementBuffer(ElementBuffer const &) = delete;
    ElementBuffer(ElementBuffer &&) = default;

    ~ElementBuffer();

    ElementBuffer &operator=(ElementBuffer const &) = delete;
    ElementBuffer &operator=(ElementBuffer &&) = default;

    [[nodiscard]] types::Size count() const;
    void bind() const;
    void unbind() const;

private:
    types::UInt _id;
    types::Size _count;
    std::shared_ptr<Window> _window;
};

} // namespace pf::gl

#endif // !ELEMENT_BUFFER_HPP
