#include <pf_utils/FileUtils.hpp>

#include <fstream>
#include <filesystem>
#include <regex>
#include <cmath>
#include <stdexcept>
#include <string>
#include <cassert>
#include <optional>

#include <fmt/core.h>

#include <pf_utils/StringUtils.hpp>

namespace pf::util::file
{

std::regex const ENDS_WITH_NUMBER("-(\\d+)$",
                                  std::regex_constants::ECMAScript | std::regex_constants::icase);

/**
 * Tries to append an alphanumeric suffix to make a unique file name.
 */
std::optional<std::filesystem::path>
uniquePathWithAlphanumericSuffix(std::filesystem::path const &existingFilePath,
                                 size_t suffixSize,
                                 size_t trialsCount)
{
    assert(!existingFilePath.empty() && existingFilePath.has_filename() &&
           std::filesystem::exists(existingFilePath));
    assert(suffixSize >= 1);

    for (size_t trialIndex = 0; trialIndex < trialsCount; trialIndex++)
    {
        std::string newFileName = fmt::format("{}-{}{}",
                                              existingFilePath.stem().string(),
                                              pf::util::string::randomAlphanumeric(suffixSize),
                                              existingFilePath.extension().string());
        std::filesystem::path newFilePath = existingFilePath;
        newFilePath.replace_filename(newFileName);

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
                            size_t trialsCount,
                            size_t startingNumber)
{
    assert(trialsCount > 0 && startingNumber < trialsCount);

    std::string fileName = existingFilePath.stem().string();
    size_t currentNumber = startingNumber;
    while (true)
    {
        std::string newFileName =
            fmt::format("{}-{}{}",
                        fileName,
                        pf::util::string::padLeftWithZeros(currentNumber, suffixSize),
                        existingFilePath.extension().string());
        std::filesystem::path newFilePath = existingFilePath;
        newFilePath.replace_filename(newFileName);

        if (!std::filesystem::exists(newFilePath))
        {
            return newFilePath;
        }
        if (currentNumber == ((startingNumber + trialsCount - 1) % trialsCount) - 1)
        {
            return std::nullopt;
        }

        currentNumber = (currentNumber + 1) % trialsCount;
    }

    return std::nullopt;
}

std::optional<std::filesystem::path> uniquePath(std::filesystem::path const &existingFilePath,
                                                size_t numericSuffixTrials,
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

    numericSuffixTrials = std::max(static_cast<size_t>(10), numericSuffixTrials);
    size_t startingNumber = 0;
    size_t suffixSize = std::floor(std::log10(numericSuffixTrials - 1)) + 1;

    // stem - filename without extension
    std::string fileName = existingFilePath.stem().string();
    // file path without a "-[0-9]{1, N}' suffix in the file name
    std::filesystem::path suffixStrippedFilePath = existingFilePath;

    std::smatch numericSuffixMatch;
    bool fileNameEndsWithNumber = std::regex_search(fileName, numericSuffixMatch, ENDS_WITH_NUMBER);
    if (fileNameEndsWithNumber)
    {
        startingNumber = std::stoi(numericSuffixMatch[1].str()) % numericSuffixTrials;
        std::string strippedFileName =
            fileName.substr(0, fileName.find_last_of('-')) + existingFilePath.extension().string();
        suffixStrippedFilePath.replace_filename(strippedFileName);
    }

    auto numericSuffixTrialResult = uniquePathWithNumericSuffix(
        suffixStrippedFilePath, suffixSize, numericSuffixTrials, startingNumber);
    if (numericSuffixTrialResult.has_value())
    {
        return numericSuffixTrialResult;
    }

    auto alphanumericSuffixTrialResult = uniquePathWithAlphanumericSuffix(
        suffixStrippedFilePath, suffixSize, alphanumericSuffixTrials);
    if (alphanumericSuffixTrialResult.has_value())
    {
        return alphanumericSuffixTrialResult;
    }

    return std::nullopt;
}

std::string readAsText(std::filesystem::path const &path)
{
    std::ifstream fileStream;
    fileStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        fileStream.open(path);
        std::ostringstream stringStream;
        stringStream << fileStream.rdbuf();
        return stringStream.str();
    }
    catch (std::ifstream::failure const &e)
    {
        throw std::runtime_error(fmt::format(
            "Error while reading shader source code ({}).\nDetails: {}.", path.string(), e.what()));
    }
}

} // namespace pf::util::file
