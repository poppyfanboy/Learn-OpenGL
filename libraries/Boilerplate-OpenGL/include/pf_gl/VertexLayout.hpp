#ifndef VERTEX_LAYOUT_HPP
#define VERTEX_LAYOUT_HPP

#include <vector>
#include <string>
#include <array>

#include <pf_gl/ValueTypes.hpp>

namespace pf::gl
{

// * Types *

enum Attribute
{
    POSITION,
    NORMAL,
    TEXTURE_COORDINATES,
    COLOR,
};

/**
 * A single atomic element of the vertex layout.
 */
struct AttributeEntry
{
    Attribute attribute;
    types::ValueType valueType;
    bool normalized;

    AttributeEntry(types::ValueType valueType, Attribute attribute, bool normalized = false);
};

class VertexLayout final
{
public:
    explicit VertexLayout(std::vector<AttributeEntry> const &attributes);

    [[nodiscard]] std::vector<AttributeEntry>::const_iterator begin() const noexcept;
    [[nodiscard]] std::vector<AttributeEntry>::const_iterator end() const noexcept;
    [[nodiscard]] types::BinarySize stride() const;

private:
    std::vector<AttributeEntry> _attributes;
    types::BinarySize _stride;
};


// * Utility functions *

std::string attributeName(Attribute attribute);

} // namespace pf::gl

#endif // !VERTEX_LAYOUT_HPP
