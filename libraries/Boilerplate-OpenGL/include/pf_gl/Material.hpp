#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <pf_gl/ValueTypes.hpp>

namespace pf::gl
{

struct Material
{
    types::Float shininess = 1.0F;
    types::FVec3 color = types::FVec3(0.0F, 0.0F, 0.0F);
};

} // namespace pf::gl

#endif // !MATERIAL_HPP
