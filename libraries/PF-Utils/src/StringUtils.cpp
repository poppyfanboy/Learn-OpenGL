#include <pf_utils/StringUtils.hpp>

#include <cstddef>
#include <string>
#include <random>
#include <cctype>
#include <utility>

std::string
pf::util::string::padRight(std::string const &string, size_t desiredStringSize, char fillerChar)
{
    int stringSizesDiff = static_cast<int>(desiredStringSize) - static_cast<int>(string.size());
    int fillerCharsCount = std::max(stringSizesDiff, 0);
    return string + std::string(fillerCharsCount, fillerChar);
}

std::string
pf::util::string::padLeft(std::string const &string, size_t desiredStringSize, char fillerChar)
{
    int stringSizesDiff = static_cast<int>(desiredStringSize) - static_cast<int>(string.size());
    int fillerCharsCount = std::max(stringSizesDiff, 0);
    return std::string(fillerCharsCount, fillerChar) + string;
}

std::string pf::util::string::padLeftWithZeros(size_t number, size_t desiredStringSize)
{
    return padLeft(std::to_string(number), desiredStringSize, '0');
}

template <typename RNG>
std::string pf::util::string::random(size_t size, RNG &rng, std::span<char const> const &characters)
{
    if (size == 0)
    {
        return "";
    }

    std::string resultString;
    resultString.reserve(size);

    std::uniform_int_distribution<size_t> charsDistribution(0, characters.size() - 1);
    for (size_t i = 0; i < size; i++)
    {
        resultString.push_back(characters[charsDistribution(rng)]);
    }

    return resultString;
}

template <typename RNG>
std::string pf::util::string::randomAlphanumeric(size_t size, RNG &rng)
{
    if (size == 0)
    {
        return "";
    }

    return random<RNG>(size, rng, pf::util::string::ALPHANUMERIC_CHARACTERS);
}

std::string pf::util::string::randomAlphanumeric(size_t size)
{
    std::random_device randomDevice;
    std::seed_seq seedSequence{randomDevice(), randomDevice(), randomDevice(), randomDevice()};
    std::mt19937 rng(seedSequence);
    return randomAlphanumeric(size, rng);
}
