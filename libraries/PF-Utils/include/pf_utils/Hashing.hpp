#ifndef HASHING_HPP
#define HASHING_HPP

#include <cstddef>
#include <utility>
#include <functional>

namespace pf::util
{

template <typename T, typename... Rest>
void hashCombine(size_t &seed, T const &value, Rest... rest);

struct PairHash
{
public:
    template <typename T, typename U>
    size_t operator()(std::pair<T, U> const &x) const;
};


// * Templates implementations *

template <typename T, typename... Rest>
void hashCombine(size_t &seed, T const &value, Rest... rest)
{
    std::hash<T> hasher;
    seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hashCombine(seed, rest), ...);
}

template <typename T, typename U>
size_t PairHash::operator()(std::pair<T, U> const &x) const
{
    size_t seed = 0;
    hashCombine(seed, x.first, x.second);
    return seed;
}

} // namespace pf::util

#endif // !HASHING_HPP
