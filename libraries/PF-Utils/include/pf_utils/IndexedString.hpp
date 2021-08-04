#ifndef INDEXED_STRING_HPP
#define INDEXED_STRING_HPP

#include <string>
#include <cstddef>
#include <string_view>

namespace pf::util
{

class IndexedString final
{
public:
    IndexedString(std::string const &templateString,
                  char substitutionChar = '?',
                  char fillerChar = '0');

    IndexedString(char const *templateString, char substitutionChar = '?', char fillerChar = '0');

    IndexedString(IndexedString const &) = delete;
    IndexedString(IndexedString &&) = default;

    IndexedString &operator=(IndexedString const &) = delete;
    IndexedString &operator=(IndexedString &&) = default;

    [[nodiscard]] char const *data() const;
    [[nodiscard]] size_t indexMaxSize() const;
    [[nodiscard]] bool indexFits(size_t index) const;
    [[nodiscard]] bool indexFits(std::string_view const &index) const;

    void filler(char filler);
    char const *withIndex(size_t index);
    char const *withIndex(std::string_view const &index);
    char const *withPaddedIndex(size_t index);
    char const *withPaddedIndex(std::string_view const &index);

private:
    char _filler;

    char *_indexBegin, *_indexEnd;
    size_t _indexMaxSize;

    std::string _string;
    char *_physicalStringBegin, *_physicalStringEnd;

    // Logical string
    char *_stringBegin, *_stringEnd;

    static char *fillStringWithNumber(char *stringIterator, size_t number);
    static char *fillStringWithChar(char const *start, char const *end, char filler);

    void resizeIndex(size_t newSize);
};

} // namespace pf::util

#endif // !INDEXED_STRING_HPP
