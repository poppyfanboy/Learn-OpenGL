/**
 * See `VectorMath.hpp` header for a simplier implementation which relies on GLM template vector
 * classes.
 */

#ifndef TEMPLATE_MATH_HPP
#define TEMPLATE_MATH_HPP

#include <type_traits>
#include <cmath>

#include <gcem/gcem.hpp>

namespace pf::util::tmath
{

template <typename T>
concept IsVector2 = requires(T a)
{
    a.x;
    a.y;
    requires(std::is_floating_point_v<decltype(a.x)>);
    requires(std::is_floating_point_v<decltype(a.y)>);
};

template <typename T>
concept IsVector3 = requires(T a)
{
    a.z;
    requires(std::is_floating_point_v<decltype(a.z)>);
};

template <typename T>
concept IsVector4 = requires(T a)
{
    a.w;
    requires(std::is_floating_point_v<decltype(a.w)>);
};

template <typename Vector>
requires(IsVector2<Vector>)
auto length(Vector const &vector) -> decltype(vector.x)
{
    return std::sqrt(vector.x * vector.x + vector.y * vector.y);
}

template <typename Vector>
requires(IsVector2<Vector> && IsVector3<Vector>)
auto length(Vector const &vector) -> decltype(vector.x)
{
    return std::sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
}

template <typename Vector>
requires(IsVector2<Vector> && IsVector3<Vector> && IsVector4<Vector>)
auto length(Vector const &vector) -> decltype(vector.x)
{
    return std::sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z +
                     vector.w * vector.w);
}

template <typename Float>
requires(std::is_floating_point_v<Float>)
Float moveToward(Float start, Float target, Float delta)
{
    if (delta <= Float(0))
    {
        return start;
    }
    if (delta >= std::abs(target - start))
    {
        return target;
    }
    return start + gcem::sgn(target - start) * delta;
}

template <typename Float, typename Vector>
requires(std::is_floating_point_v<Float> && IsVector2<Vector>)
Vector moveToward(Vector const &start, Vector const &target, Float delta)
{
    if (delta <= Float(0))
    {
        return start;
    }
    Vector difference = target - start;
    Float distance = length(difference);
    if (distance <= delta || distance == Float(0))
    {
        return target;
    }
    return start + difference / distance * delta;
}

} // namespace pf::util::tmath

#endif // !TEMPLATE_MATH_HPP
