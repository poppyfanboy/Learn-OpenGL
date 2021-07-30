/**
 * A collection of utility functions which operate on vectors. GLM is used as a base vector library.
 * See the `TemplateMath.hpp` header for more generic template functions which do not depend on GLM
 * library.
 */

#ifndef VECTOR_MATH_HPP
#define VECTOR_MATH_HPP

#include <glm/glm.hpp>

namespace pf::util::math
{

template <glm::length_t L, typename T, glm::qualifier Q>
GLM_FUNC_QUALIFIER glm::vec<L, T, Q>
moveToward(glm::vec<L, T, Q> const &start, glm::vec<L, T, Q> const &target, T delta)
{
    if (delta <= T(0))
    {
        return start;
    }
    glm::vec<L, T, Q> difference = target - start;
    T distance = glm::length(difference);
    if (distance <= delta || distance == T(0))
    {
        return target;
    }
    return start + difference / distance * delta;
}

} // namespace pf::util::math

#endif // !VECTOR_MATH_HPP
