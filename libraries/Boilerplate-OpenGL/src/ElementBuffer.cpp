#include <pf_gl/ElementBuffer.hpp>

#include <utility>
#include <stdexcept>
#include <memory>
#include <span>

#include <gsl/util>

#include <pf_gl/RenderingOptions.hpp>
#include <pf_gl/Window.hpp>
#include <pf_gl/ValueTypes.hpp>
#include <pf_utils/RawBuffer.hpp>

namespace pf::gl
{

ElementBuffer::ElementBuffer(std::shared_ptr<Window> window,
                             std::span<const types::UInt> const &indices,
                             UsagePattern usagePattern)
    : _window(std::move(window))
    , _id(0)
    , _count(gsl::narrow_cast<types::Size>(indices.size()))
{
    GLenum glUsage = usagePatternToGLenum(usagePattern);

    _window->bindContext();
    glGenBuffers(1, &_id);
    if (_id == 0)
    {
        throw std::runtime_error("Failed to generate an element buffer.");
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 gsl::narrow_cast<types::BinarySize>(indices.size_bytes()),
                 indices.data(),
                 glUsage);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

ElementBuffer::~ElementBuffer()
{
    _window->bindContext();
    glDeleteBuffers(1, &_id);
}

void ElementBuffer::bind() const
{
    _window->bindContext();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _id);
}

types::Size ElementBuffer::count() const
{
    return _count;
}

void ElementBuffer::unbind() const
{
    _window->bindContext();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

} // namespace pf::gl
