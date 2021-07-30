#include <pf_gl/VertexArray.hpp>

#include <memory>
#include <utility>
#include <stdexcept>

#include <glad/glad.h>
#include <gsl/narrow>

#include <pf_gl/ValueTypes.hpp>
#include <pf_gl/Window.hpp>

namespace pf::gl
{

VertexArray::VertexArray(std::shared_ptr<Window> window)
    : _window(std::move(window))
    , _id(0)
{
    _window->bindContext();

    glGenVertexArrays(1, &_id);
    if (_id == 0)
    {
        throw std::runtime_error("Failed to generate a vertex array.");
    }

    glBindVertexArray(0);
}

VertexArray::~VertexArray()
{
    _window->bindContext();
    glDeleteVertexArrays(1, &_id);
}

void VertexArray::bind() const
{
    _window->bindContext();
    glBindVertexArray(_id);
}

void VertexArray::addVertexBuffer(std::shared_ptr<VertexBuffer> const &vertexBuffer)
{
    _window->bindContext();

    _vertexBuffers.push_back(vertexBuffer);

    this->bind();
    vertexBuffer->bind();

    types::UInt attributeIndex = 0;
    types::BinarySize currentOffset = 0;
    VertexLayout const &vertexLayout = vertexBuffer->layout();

    for (auto const &attribute : vertexLayout)
    {
        glVertexAttribPointer(attributeIndex,
                              types::scalarCount(attribute.valueType),
                              types::openglScalar(attribute.valueType),
                              attribute.normalized ? GL_TRUE : GL_FALSE,
                              gsl::narrow_cast<types::Size>(vertexLayout.stride()),
                              reinterpret_cast<GLvoid const *>(currentOffset));
        glEnableVertexAttribArray(attributeIndex);

        currentOffset += types::sizeInBytes(attribute.valueType);
        attributeIndex++;
    }

    this->unbind();
    vertexBuffer->unbind();
}

void VertexArray::setElementBuffer(std::shared_ptr<ElementBuffer> const &elementBuffer)
{
    _window->bindContext();

    _elementBuffer = elementBuffer;

    this->bind();
    elementBuffer->bind();
    this->unbind();
    elementBuffer->unbind();
}

void VertexArray::draw()
{
    bind();
    glDrawElements(GL_TRIANGLES, _elementBuffer->count(), GL_UNSIGNED_INT, nullptr);
}

void VertexArray::unbind() const
{
    _window->bindContext();
    glBindVertexArray(0);
}

} // namespace pf::gl
