/**
 * At first I thought that templating such functions is a nice idea, but it turned out to be a
 * disaster to implement event a simple template function which operates on vectors. See
 * `VectorMath.hpp` header for a simplier implementation which relies on GLM template vector
 * classes.
 */

#ifndef TEMPLATE_MATH_HPP
#define TEMPLATE_MATH_HPP

#include <type_traits>

#include <gcem/gcem.hpp>

namespace pf::util::tmath
{

// * Magic SFINAE stuff *

/**
 * Checks if an object has the specified field.
 * Explanation of how this works: https://stackoverflow.com/a/16000226
 */
// NOLINTNEXTLINE
#define HAS_MEMBER(CheckName, MemberName)                                                          \
    template <typename T, typename = int>                                                          \
    struct CheckName : std::false_type                                                             \
    {                                                                                              \
    };                                                                                             \
                                                                                                   \
    template <typename T>                                                                          \
    struct CheckName<T, decltype((void)T::MemberName, 0)> : std::true_type                         \
    {                                                                                              \
    };

// Used for detecting the number of components in a vector.
HAS_MEMBER(hasX, x)
HAS_MEMBER(hasY, y)
HAS_MEMBER(hasZ, z)
HAS_MEMBER(hasW, w)


// * Functions implementations *

template <
    typename Float,
    typename Vector,
    std::enable_if_t<hasX<Vector>::value && hasY<Vector>::value && !hasZ<Vector>::value, int> = 0>
Float length(Vector const &vector)
{
    return gcem::sqrt(vector.x * vector.x + vector.y * vector.y);
}

template <typename Float,
          typename Vector,
          std::enable_if_t<hasX<Vector>::value && hasY<Vector>::value && hasZ<Vector>::value &&
                               !hasW<Vector>::value,
                           int> = 0>
Float length(Vector const &vector)
{
    return gcem::sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
}

template <typename Float,
          typename Vector,
          std::enable_if_t<hasX<Vector>::value && hasY<Vector>::value && hasZ<Vector>::value &&
                               hasW<Vector>::value,
                           int> = 0>
Float length(Vector const &vector)
{
    return gcem::sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z +
                      vector.w * vector.w);
}

template <typename Float>
Float moveToward(Float start, Float target, Float delta)
{
    if (delta <= Float(0))
    {
        return start;
    }
    if (delta >= gcem::abs(target - start))
    {
        return target;
    }
    return start + gcem::sgn(target - start) * delta;
}

template <typename Float, typename Vector>
Vector moveToward(Vector const &start, Vector const &target, Float delta)
{
    if (delta <= Float(0))
    {
        return start;
    }
    Vector difference = target - start;
    Float distance = length<Float, Vector>(difference);
    if (distance <= delta || distance == Float(0))
    {
        return target;
    }
    return start + difference / distance * delta;
}

} // namespace pf::util::tmath

#endif // !TEMPLATE_MATH_HPP
