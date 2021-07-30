#ifndef VERTEX_BUFFER_HPP
#define VERTEX_BUFFER_HPP

#include <memory>

#include <pf_gl/VertexLayout.hpp>
#include <pf_gl/RenderingOptions.hpp>
#include <pf_gl/Window.hpp>
#include <pf_gl/ValueTypes.hpp>
#include <pf_utils/RawBuffer.hpp>

namespace pf::gl
{

class VertexBuffer final
{
public:
    VertexBuffer(std::shared_ptr<Window> window,
                 pf::util::RawBuffer const &data,
                 UsagePattern usagePattern,
                 VertexLayout layout);

    VertexBuffer(VertexBuffer const &) = delete;
    VertexBuffer(VertexBuffer &&) = default;

    ~VertexBuffer();

    VertexBuffer &operator=(VertexBuffer const &) = delete;
    VertexBuffer &operator=(VertexBuffer &&) = default;

    void bind() const;
    void unbind() const;
    [[nodiscard]] VertexLayout layout() const;

private:
    types::UInt _id;
    VertexLayout _layout;
    std::shared_ptr<Window> _window;
};

} // namespace pf::gl

#endif // !VERTEX_BUFFER_HPP
