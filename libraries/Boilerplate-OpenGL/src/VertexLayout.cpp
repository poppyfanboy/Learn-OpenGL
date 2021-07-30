#include <pf_gl/VertexLayout.hpp>

#include <string>

#include <sparsepp/spp.h>

#include <pf_gl/ValueTypes.hpp>

namespace pf::gl
{

spp::sparse_hash_map<Attribute, std::string> const ATTRIBUTES_NAMES{
    {POSITION, "Position"},
    {NORMAL, "Normal"},
    {TEXTURE_COORDINATES, "Texture coordinates"},
    {COLOR, "Color"},
};

AttributeEntry::AttributeEntry(types::ValueType valueType, Attribute attribute, bool normalized)
    : valueType(valueType)
    , attribute(attribute)
    , normalized(normalized)
{
}

VertexLayout::VertexLayout(std::vector<AttributeEntry> const &attributes)
    : _attributes(attributes)
{
    _stride = 0;
    for (auto const &attribute : attributes)
    {
        _stride += types::sizeInBytes(attribute.valueType);
    }
}

std::vector<AttributeEntry>::const_iterator VertexLayout::begin() const noexcept
{
    return _attributes.cbegin();
}

std::vector<AttributeEntry>::const_iterator VertexLayout::end() const noexcept
{
    return _attributes.cend();
}

types::BinarySize VertexLayout::stride() const
{
    return _stride;
}

std::string attributeName(Attribute attribute)
{
    return ATTRIBUTES_NAMES.at(attribute);
}

} // namespace pf::gl
