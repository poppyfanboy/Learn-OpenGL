#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP

#include <string>
#include <span>

namespace pf::util::string
{

static std::string const ALPHANUMERIC_CHARACTERS_STRING =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

std::span<char const> const ALPHANUMERIC_CHARACTERS =
    std::span<char const>(ALPHANUMERIC_CHARACTERS_STRING.begin(),
                          ALPHANUMERIC_CHARACTERS_STRING.end());

/**
 * Adds filler characters at the end of the passed string so that it would be `resultStringSize`
 * characters long. (Returns the passed string in case it is already `resultStringSize` characters
 * long.)
 */
std::string padRight(std::string const &string, size_t desiredStringSize, char fillerChar = ' ');

/**
 * Same thing as `padRight` function, but it adds the padding characters to the begining of the
 * string.
 * @see padRight
 */
std::string padLeft(std::string const &string, size_t desiredStringSize, char fillerChar = ' ');

/**
 * Adds zeros to the begining of the number.
 * @see padleft
 */
std::string padLeftWithZeros(size_t number, size_t desiredStringSize);

/**
 * Generates a random string of length `size` consiting of the characters from the `characters`
 * span.
 */
template <typename RNG>
std::string random(size_t size, RNG &rng, std::span<char const> const &characters);

/**
 * Generates a random alphanumeric string of size `size`.
 * @see pf::util::string::random
 */
template <typename RNG>
std::string randomAlphanumeric(size_t size, RNG &rng);

/**
 * Generates a string of alphanumeric characters using `std::mt19937` with `random_device`-generated
 * initial seed.
 * @see pf::util::string::random
 */
std::string randomAlphanumeric(size_t size);

} // namespace pf::util::string

#endif // !STRING_UTILS_HPP
