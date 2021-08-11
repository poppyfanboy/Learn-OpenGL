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
#include <ranges>

#include <fmt/format.h>

#include <pf_utils/StringUtils.hpp>
#include <pf_utils/Math.hpp>
#include <pf_utils/RangeAlgorithms.hpp>

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

std::filesystem::path appendSuffix(std::filesystem::path const &path, std::string const &suffix);

std::filesystem::path appendSuffix(std::filesystem::path const &path, size_t suffix);

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

    size_t numericSuffixSize = pf::util::math::digitsCount(moduloNumber - 1);
    for (size_t suffix = 0; suffix < moduloNumber; suffix++)
    {
        auto newFilePath = appendSuffix(
            suffixStrippedFilePath, pf::util::string::padLeftWithZeros(suffix, numericSuffixSize));

        if (!std::filesystem::exists(newFilePath))
        {
            return newFilePath;
        }
    }

    size_t alphanumericSuffixSize = moduloNumber > 1 ? pf::util::math::digitsCount(moduloNumber - 1)
                                                     : ALPHANUMERIC_SUFFIX_DEFAULT_SIZE;
    for (size_t suffixIndex = 0; suffixIndex < alphanumericSuffixTrials; suffixIndex++)
    {
        auto newFilePath = appendSuffix(
            existingFilePath, pf::util::string::randomAlphanumeric(alphanumericSuffixSize));

        if (!std::filesystem::exists(newFilePath))
        {
            return newFilePath;
        }
    }

    return std::nullopt;
}

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

    auto uniqueSuffix = pf::util::exponentialSearch(
        std::ranges::views::iota(static_cast<size_t>(0), MAX_NUMERIC_SUFFIX),
        [](auto &&path) { return std::filesystem::exists(path); },
        [existingFilePath](size_t suffix) { return appendSuffix(existingFilePath, suffix); });

    if (*uniqueSuffix == MAX_NUMERIC_SUFFIX)
    {
        return uniquePathSequential(existingFilePath);
    }
    return appendSuffix(existingFilePath, *uniqueSuffix);
}

std::string readAsText(std::filesystem::path const &path)
{
    std::ifstream fileStream;
    fileStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        fileStream.open(path, std::ifstream::in | std::ifstream::binary);

        fileStream.seekg(0, std::ios::end);
        std::streamsize fileSize = fileStream.tellg();
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

} // namespace pf::util::file
