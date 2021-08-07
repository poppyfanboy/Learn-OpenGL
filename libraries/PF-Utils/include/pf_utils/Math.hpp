#ifndef MATH_HPP
#define MATH_HPP

#include <cstddef>
#include <type_traits>
#include <cmath>
#include <limits>
#include <string>
#include <array>
#include <bit>
#include <climits>

namespace pf::util::math
{

/**
 * Integer power (recursive, logarithmic complexity).
 */
template <typename T>
requires(std::is_integral_v<T>)
T pow(T base, size_t exponent);

/**
 * Trivial implementation, O(digitsCount) complexity.
 */
template <typename T>
requires(std::is_integral_v<T>)
size_t digitsCountIterative(T number);

/**
 * http://graphics.stanford.edu/~seander/bithacks.html#IntegerLog10
 */
template <typename T>
requires(std::is_integral_v<T>)
size_t digitsCount(T number);

template <typename T>
requires(std::is_integral_v<T>)
bool isPoT(T number);


// * Template functions implementations *

static std::array<uint64_t, 20> const POWERS_OF_TEN{
    1U,
    10U,
    100U,
    1000U,
    10000U,
    100000U,
    1000000U,
    10000000U,
    100000000U,
    1000000000U,
    10000000000U,
    100000000000U,
    1000000000000U,
    10000000000000U,
    100000000000000U,
    1000000000000000U,
    10000000000000000U,
    100000000000000000U,
    1000000000000000000U,
    10000000000000000000U,
};

template <typename T>
requires(std::is_integral_v<T>)
T pow(T base, size_t exponent)
{
    if (exponent == 0)
    {
        return T(1);
    }
    if (exponent == 1)
    {
        return base;
    }

    T squareRootOfResult = pow(base, exponent / 2);
    if (exponent % 2 == 0)
    {
        return squareRootOfResult * squareRootOfResult;
    }
    return base * squareRootOfResult * squareRootOfResult;
}

template <typename T>
requires(std::is_integral_v<T>)
size_t digitsCountIterative(T number)
{
    number += static_cast<T>(number == T(0));

    if constexpr (std::is_signed_v<T>)
    {
        if (number == std::numeric_limits<T>::min())
        {
            number = std::numeric_limits<T>::max();
        }
        if (number < 0)
        {
            number = -number;
        }
    }

    size_t digitsCount = 1;
    while (number >= 1000)
    {
        digitsCount += 3;
        number /= 1000;
    }
    while (number >= 10)
    {
        digitsCount++;
        number /= 10;
    }

    return digitsCount;
}

template <typename T>
requires(std::is_integral_v<T>)
size_t digitsCount(T number)
{
    if constexpr (sizeof(T) > 8)
    {
        return digitsCountIterative(number);
    }

    using UnsignedT = typename std::make_unsigned<T>::type;

    number += static_cast<T>(number == T(0));

    if constexpr (std::is_signed_v<T>)
    {
        if (number == std::numeric_limits<T>::min())
        {
            number = std::numeric_limits<T>::max();
        }
        if (number < 0)
        {
            number = -number;
        }
    }

    T digitsCountEstimate =
        ((sizeof(T) * CHAR_BIT - std::countl_zero(static_cast<UnsignedT>(number)) + 1) * 1233) >>
        12;

    return 1 + digitsCountEstimate - static_cast<T>(number < POWERS_OF_TEN[digitsCountEstimate]);
}

template <typename T>
requires(std::is_integral_v<T>)
bool isPoT(T number)
{
    if constexpr (std::is_signed_v<T>)
    {
        if (number == std::numeric_limits<T>::min())
        {
            return true;
        }
        number = -number;
    }

    return number > 0 && (number & (number - 1)) == 0;
}

} // namespace pf::util::math

#endif // !MATH_HPP
