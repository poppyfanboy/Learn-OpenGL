#ifndef VERTEX_ARRAY_HPP
#define VERTEX_ARRAY_HPP

#include <memory>
#include <vector>

#include <pf_gl/VertexBuffer.hpp>
#include <pf_gl/ElementBuffer.hpp>
#include <pf_gl/Window.hpp>
#include <pf_gl/ValueTypes.hpp>

namespace pf::gl
{

class VertexArray final
{
public:
    explicit VertexArray(std::shared_ptr<Window> window);

    VertexArray(VertexArray const &) = delete;
    VertexArray(VertexArray &&) = default;

    ~VertexArray();

    VertexArray &operator=(VertexArray const &) = delete;
    VertexArray &operator=(VertexArray &&) = default;

    void bind() const;
    void unbind() const;

    void addVertexBuffer(std::shared_ptr<VertexBuffer> const &vertexBuffer);
    void setElementBuffer(std::shared_ptr<ElementBuffer> const &elementBuffer);
    void draw();

private:
    std::shared_ptr<Window> _window;
    types::UInt _id;
    std::vector<std::shared_ptr<VertexBuffer>> _vertexBuffers;
    std::shared_ptr<ElementBuffer> _elementBuffer;
};

} // namespace pf::gl

#endif // !VERTEX_ARRAY_HPP
