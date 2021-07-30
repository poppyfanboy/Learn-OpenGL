#include <pf_gl/VertexBuffer.hpp>

#include <stdexcept>
#include <utility>
#include <memory>

#include <gsl/util>

#include <pf_gl/RenderingOptions.hpp>
#include <pf_gl/Window.hpp>
#include <pf_utils/RawBuffer.hpp>

namespace pf::gl
{

VertexBuffer::VertexBuffer(std::shared_ptr<Window> window,
                           pf::util::RawBuffer const &data,
                           UsagePattern usagePattern,
                           VertexLayout layout)
    : _window(std::move(window))
    , _layout(std::move(layout))
    , _id(0)
{
    _window->bindContext();

    glGenBuffers(1, &_id);
    if (_id == 0)
    {
        throw std::runtime_error("Failed to generate a vertex buffer.");
    }
    glBindBuffer(GL_ARRAY_BUFFER, _id);

    GLenum glUsagePattern = usagePatternToGLenum(usagePattern);
    glBufferData(GL_ARRAY_BUFFER,
                 gsl::narrow_cast<types::Size>(data.size()),
                 data.pointer(),
                 glUsagePattern);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

VertexBuffer::~VertexBuffer()
{
    _window->bindContext();
    glDeleteBuffers(1, &_id);
}

void VertexBuffer::bind() const
{
    _window->bindContext();
    glBindBuffer(GL_ARRAY_BUFFER, _id);
}

VertexLayout VertexBuffer::layout() const
{
    return _layout;
}

void VertexBuffer::unbind() const
{
    _window->bindContext();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

} // namespace pf::gl
