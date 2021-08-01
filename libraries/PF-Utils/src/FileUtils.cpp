#include <pf_utils/FileUtils.hpp>

#include <fstream>
#include <ios>
#include <filesystem>
#include <regex>
#include <cmath>
#include <stdexcept>
#include <string>
#include <cassert>
#include <cstddef>
#include <optional>
#include <limits>

#include <fmt/core.h>

#include <pf_utils/StringUtils.hpp>
#include <pf_utils/Math.hpp>

namespace pf::util::file
{

std::regex const ENDS_WITH_NUMBER("-(\\d+)$", std::regex_constants::ECMAScript);

size_t const ALPHANUMERIC_SUFFIX_DEFAULT_SIZE = 10;
size_t const MAX_NUMERIC_SUFFIX = std::numeric_limits<size_t>::max() / 2;

/**
 * Tries to append an alphanumeric suffix to the existing file name to make a unique file name
 * inside the scope of the parent folder.
 */
std::optional<std::filesystem::path>
uniquePathWithAlphanumericSuffix(std::filesystem::path const &existingFilePath,
                                 size_t suffixSize,
                                 size_t trialsCount);

/**
 * Tries suffixes from `startingNumber` to `moduloNumber - 1` and then from 0 to
 * `startingNumber - 1`.
 */
std::optional<std::filesystem::path>
uniquePathWithNumericSuffix(std::filesystem::path const &existingFilePath,
                            size_t suffixSize,
                            size_t moduloNumber,
                            size_t startingNumber);

/**
 * Removes a numeric suffix from the file name.
 * (Converts "path/to/file-23.txt" to "path/to/file.txt".)
 */
std::filesystem::path stripNumericSuffix(std::filesystem::path const &path);

std::optional<std::filesystem::path>
uniquePathSequential(std::filesystem::path const &existingFilePath,
                     size_t moduloNumber,
                     size_t alphanumericSuffixTrials)
{
    if (existingFilePath.empty() || !existingFilePath.has_filename())
    {
        throw std::runtime_error(fmt::format("Invalid path: \"{}\".", existingFilePath.string()));
    }
    if (!std::filesystem::exists(existingFilePath))
    {
        return existingFilePath;
    }

    // If a filename has a numeric suffix, remove it to avoid layering suffixes on one another
    std::filesystem::path suffixStrippedFilePath = stripNumericSuffix(existingFilePath);

    if (moduloNumber != 0)
    {
        size_t suffixSize = pf::util::math::digitsCount(moduloNumber - 1);
        auto numericSuffixTrialResult =
            uniquePathWithNumericSuffix(suffixStrippedFilePath, suffixSize, moduloNumber, 0);

        if (numericSuffixTrialResult.has_value())
        {
            return numericSuffixTrialResult;
        }
    }

    if (alphanumericSuffixTrials != 0)
    {
        size_t suffixSize = moduloNumber > 1 ? pf::util::math::digitsCount(moduloNumber - 1)
                                             : ALPHANUMERIC_SUFFIX_DEFAULT_SIZE;
        auto alphanumericSuffixTrialResult = uniquePathWithAlphanumericSuffix(
            suffixStrippedFilePath, suffixSize, alphanumericSuffixTrials);

        if (alphanumericSuffixTrialResult.has_value())
        {
            return alphanumericSuffixTrialResult;
        }
    }

    return std::nullopt;
}

std::filesystem::path appendSuffix(std::filesystem::path const &path, std::string const &suffix);

std::filesystem::path appendSuffix(std::filesystem::path const &path, size_t suffix);

std::optional<size_t> uniqueSuffixExponentialSearch(std::filesystem::path const &path);

std::optional<size_t>
uniqueSuffixBinarySearch(std::filesystem::path const &originalPath, size_t low, size_t high);

std::optional<std::filesystem::path> uniquePath(std::filesystem::path const &existingFilePath)
{
    if (existingFilePath.empty() || !existingFilePath.has_filename())
    {
        throw std::runtime_error(fmt::format("Invalid path: \"{}\".", existingFilePath.string()));
    }
    if (!std::filesystem::exists(existingFilePath))
    {
        return existingFilePath;
    }

    auto maxSuffixSearchResult = uniqueSuffixExponentialSearch(existingFilePath);
    if (!maxSuffixSearchResult.has_value())
    {
        // Fallback to sequential search
        return uniquePathSequential(existingFilePath);
    }
    size_t maxSuffix = maxSuffixSearchResult.value();

    auto minSuffixSearchResult = uniqueSuffixBinarySearch(existingFilePath, 0, maxSuffix);
    if (!minSuffixSearchResult.has_value())
    {
        assert(!std::filesystem::exists(appendSuffix(existingFilePath, maxSuffix)));

        // Just use the suffix found with exponential search in case binary search fails
        return appendSuffix(existingFilePath, maxSuffix);
    }

    return appendSuffix(existingFilePath, minSuffixSearchResult.value());
}

std::string readAsText(std::filesystem::path const &path)
{
    std::ifstream fileStream;
    fileStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        fileStream.open(path, std::ifstream::in | std::ifstream::binary);

        fileStream.ignore(std::numeric_limits<std::streamsize>::max());
        std::streamsize fileSize = fileStream.gcount();
        fileStream.clear();
        fileStream.seekg(0, std::fstream::beg);

        std::string readText(fileSize, ' ');
        fileStream.read(&readText[0], fileSize);

        return readText;
    }
    catch (std::ifstream::failure const &e)
    {
        throw std::runtime_error(fmt::format(
            "Error while reading shader source code ({}).\nDetails: {}.", path.string(), e.what()));
    }
}


// * Utility functions implementations *

std::filesystem::path appendSuffix(std::filesystem::path const &path, std::string const &suffix)
{
    std::string newFileName =
        fmt::format("{}-{}{}", path.stem().string(), suffix, path.extension().string());
    return std::filesystem::path(path).replace_filename(newFileName);
}

std::filesystem::path appendSuffix(std::filesystem::path const &path, size_t suffix)
{
    return appendSuffix(path, std::to_string(suffix));
}

std::filesystem::path stripNumericSuffix(std::filesystem::path const &path)
{
    auto suffixStrippedPath = path;

    // stem - filename without extension
    std::string fileName = path.stem().string();

    std::smatch numericSuffixMatch;
    bool fileNameEndsWithNumber = std::regex_search(fileName, numericSuffixMatch, ENDS_WITH_NUMBER);
    if (fileNameEndsWithNumber)
    {
        std::string strippedFileName =
            fileName.substr(0, fileName.find_last_of('-')) + path.extension().string();
        suffixStrippedPath.replace_filename(strippedFileName);
    }

    return suffixStrippedPath;
}

std::optional<std::filesystem::path>
uniquePathWithAlphanumericSuffix(std::filesystem::path const &existingFilePath,
                                 size_t suffixSize,
                                 size_t trialsCount)
{
    assert(!existingFilePath.empty() && existingFilePath.has_filename());
    assert(std::filesystem::exists(existingFilePath));
    assert(suffixSize >= 1);

    for (size_t trialIndex = 0; trialIndex < trialsCount; trialIndex++)
    {
        auto newFilePath =
            appendSuffix(existingFilePath, pf::util::string::randomAlphanumeric(suffixSize));

        if (!std::filesystem::exists(newFilePath))
        {
            return newFilePath;
        }
    }

    return std::nullopt;
}

std::optional<std::filesystem::path>
uniquePathWithNumericSuffix(std::filesystem::path const &existingFilePath,
                            size_t suffixSize,
                            size_t moduloNumber,
                            size_t startingNumber)
{
    assert(!existingFilePath.empty() && existingFilePath.has_filename());
    assert(std::filesystem::exists(existingFilePath));
    assert(moduloNumber != 0);
    assert(startingNumber < moduloNumber);
    assert(suffixSize >= 1);

    std::string fileName = existingFilePath.stem().string();
    size_t currentNumber = startingNumber;
    while (true)
    {
        auto newFilePath = appendSuffix(
            existingFilePath, pf::util::string::padLeftWithZeros(currentNumber, suffixSize));

        if (!std::filesystem::exists(newFilePath))
        {
            return newFilePath;
        }
        if (currentNumber == (startingNumber + moduloNumber - 1) % moduloNumber)
        {
            return std::nullopt;
        }

        currentNumber = (currentNumber + 1) % moduloNumber;
    }

    return std::nullopt;
}

std::optional<size_t> uniqueSuffixExponentialSearch(std::filesystem::path const &path)
{
    assert(!path.empty() && path.has_filename());
    assert(std::filesystem::exists(path));

    // Find the largest suffix which have not been used yet
    size_t suffix = 0;
    while (true)
    {
        if (!std::filesystem::exists(appendSuffix(path, suffix)))
        {
            return suffix;
        }
        // Next iteration is going to overflow the suffix variable
        if (suffix > (MAX_NUMERIC_SUFFIX - 1) / 2)
        {
            if (!std::filesystem::exists(appendSuffix(path, MAX_NUMERIC_SUFFIX)))
            {
                return MAX_NUMERIC_SUFFIX;
            }
            return std::nullopt;
        }
        suffix = 2 * suffix + 1;
    }

    return std::nullopt;
}

std::optional<size_t>
uniqueSuffixBinarySearch(std::filesystem::path const &path, size_t low, size_t high)
{
    assert(low < std::numeric_limits<size_t>::max() / 2);
    assert(high < std::numeric_limits<size_t>::max() / 2);

    while (low < high)
    {
        size_t middle = (low + high) / 2;
        auto suffixedPath = appendSuffix(path, middle);
        if (std::filesystem::exists(suffixedPath))
        {
            low = middle + 1;
        }
        else
        {
            high = middle;
        }
    }
    if (std::filesystem::exists(appendSuffix(path, low)))
    {
        return std::nullopt;
    }
    return low;
}

} // namespace pf::util::file
