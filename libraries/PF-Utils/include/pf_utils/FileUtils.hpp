#ifndef FILE_UTILS_HPP
#define FILE_UTILS_HPP

#include <filesystem>
#include <cstddef>
#include <optional>

namespace pf::util::file
{

/**
 * Tries to generate a unique file name (based on the passed file path) by adding a numerical
 * suffix. In case it fails, it tries to add a unique suffix consisting of random alphanumeric
 * characters.
 * @param moduloNumber How many files with names like "file-%number%" there can be in the
 * parent folder.
 * @param alphanumericSuffixTrials How many times the function will try to generate an alphanumeric
 * suffix to make a unique file name until it gives up and returns a `nullopt`.
 */
std::optional<std::filesystem::path>
uniquePathSequential(std::filesystem::path const &existingFilePath,
                     size_t moduloNumber = 100,
                     size_t alphanumericSuffixTrials = 5);

std::optional<std::filesystem::path> uniquePath(std::filesystem::path const &existingFilePath);

std::string readAsText(std::filesystem::path const &path);

} // namespace pf::util::file

#endif // !FILE_UTILS_HPP
