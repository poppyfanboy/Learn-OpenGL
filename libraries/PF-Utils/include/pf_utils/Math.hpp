#ifndef MATH_HPP
#define MATH_HPP

#include <cstddef>
#include <type_traits>
#include <cmath>
#include <limits>
#include <string>
#include <array>

namespace pf::util::math
{

std::array<uint32_t, 10> const SIZES_TABLE{9,
                                           99,
                                           999,
                                           9999,
                                           99999,
                                           999999,
                                           9999999,
                                           99999999,
                                           999999999,
                                           std::numeric_limits<uint32_t>::max()};

template <typename T, std::enable_if_t<std::numeric_limits<T>::is_integer, int> = 0>
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

template <typename T, std::enable_if_t<std::numeric_limits<T>::is_integer, int> = 0>
size_t digitsCountIterative(T number)
{
    T quotient = number;
    size_t digitsCount = 0;
    while (quotient != 0)
    {
        digitsCount++;
        quotient /= 10;
    }
    return digitsCount;
}

template <typename T, std::enable_if_t<std::numeric_limits<T>::is_integer, int> = 0>
size_t digitsCount(T number)
{
    if (number == 0)
    {
        return 1;
    }

    if constexpr (std::is_signed_v<T>)
    {
        if (number < 0 && number > std::numeric_limits<T>::min())
        {
            number = -number;
        }
        else
        {
            return digitsCountIterative(number);
        }
    }

    for (size_t i = 0; i < SIZES_TABLE.size(); i++)
    {
        if (number <= SIZES_TABLE[i])
        {
            return i + 1;
        }
    }

    return digitsCountIterative(number);
}

template <typename T, std::enable_if_t<std::numeric_limits<T>::is_integer, int> = 0>
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
