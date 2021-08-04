#include <pf_utils/IndexedString.hpp>

#include <stdexcept>
#include <string>
#include <cstring>
#include <cassert>

#include <gsl/util>

#include <pf_utils/Math.hpp>

namespace pf::util
{

IndexedString::IndexedString(std::string const &templateString,
                             char substitutionChar,
                             char fillerChar)
    : IndexedString(templateString.c_str(), substitutionChar, fillerChar)
{
}

IndexedString::IndexedString(char const *templateString, char substitutionChar, char fillerChar)
    : _string(templateString)
    , _filler(fillerChar)
{
    _physicalStringBegin = const_cast<char *>(_string.c_str());
    _stringBegin = _physicalStringBegin;

    _physicalStringEnd = const_cast<char *>(_string.c_str() + _string.size() + 1);
    _stringEnd = _physicalStringEnd;

    if (substitutionChar == '\0' || fillerChar == '\0')
    {
        throw std::invalid_argument(
            "Null character cannot be used as a substitution or filler character");
    }

    _indexBegin = const_cast<char *>(std::strchr(_string.c_str(), substitutionChar));
    if (*_indexBegin == '\0')
    {
        throw std::invalid_argument("Specified string does not have any substitution characters");
    }
    *_indexBegin = fillerChar;

    char *stringIterator = _indexBegin + 1;
    _indexMaxSize = 1;
    while (*stringIterator == substitutionChar)
    {
        _indexMaxSize++;
        stringIterator++;
    }
    _indexEnd = _indexBegin + _indexMaxSize - 1;
}

[[nodiscard]] char const *IndexedString::data() const
{
    return _stringBegin;
}

[[nodiscard]] size_t IndexedString::indexMaxSize() const
{
    return _indexMaxSize;
}

[[nodiscard]] bool IndexedString::indexFits(size_t index) const
{
    return pf::util::math::digitsCount(index) <= _indexMaxSize;
}

[[nodiscard]] bool IndexedString::indexFits(std::string_view const &index) const
{
    return index.size() <= _indexMaxSize;
}

void IndexedString::filler(char filler)
{
    _filler = filler;
}

char const *IndexedString::withIndex(size_t index)
{
    size_t digitsCount = pf::util::math::digitsCount(index);
    resizeIndex(digitsCount);
    fillStringWithNumber(_indexBegin, index);

    return data();
}

char const *IndexedString::withIndex(std::string_view const &index)
{
    resizeIndex(index.size());
    std::memcpy(_indexBegin, index.data(), index.size());

    return data();
}

char const *IndexedString::withPaddedIndex(size_t index)
{
    size_t digitsCount = pf::util::math::digitsCount(index);
    resizeIndex(_indexMaxSize);

    char *logicalIndexStart = _indexBegin + (_indexMaxSize - digitsCount);
    fillStringWithChar(_indexBegin, logicalIndexStart, _filler);
    fillStringWithNumber(logicalIndexStart, index);

    return data();
}

char const *IndexedString::withPaddedIndex(std::string_view const &index)
{
    resizeIndex(_indexMaxSize);

    char *logicalIndexStart = _indexBegin + (_indexMaxSize - index.size());
    fillStringWithChar(_indexBegin, logicalIndexStart, _filler);
    std::memcpy(logicalIndexStart, index.data(), index.size());

    return data();
}

char *IndexedString::fillStringWithNumber(char *stringIterator, size_t number)
{
    size_t digitsCount = pf::util::math::digitsCount(number);
    size_t cutNumber = number;
    size_t divisor = pf::util::math::pow(10, digitsCount - 1);

    for (size_t i = 1; i <= digitsCount; i++)
    {
        size_t digit = cutNumber / divisor;

        *(stringIterator++) = gsl::narrow_cast<char>(digit + '0');
        cutNumber -= digit * divisor;
        divisor /= 10;
    }

    return stringIterator;
}

char *IndexedString::fillStringWithChar(char const *start, char const *end, char filler)
{
    char *stringIterator = const_cast<char *>(start);
    while (stringIterator < end)
    {
        *(stringIterator++) = filler;
    }

    return stringIterator;
}

void IndexedString::resizeIndex(size_t newSize)
{
    assert(newSize > 0 && newSize <= _indexMaxSize);

    if (newSize > _indexMaxSize)
    {
        throw std::invalid_argument("Given index is too large.");
    }

    if (_indexEnd - _indexBegin + 1 != newSize)
    {
        bool canFitDigits = false;

        // Index is right at the begining / end of the string
        if (_indexBegin == _stringBegin)
        {
            _indexBegin = _indexEnd - newSize + 1;
            _stringBegin = _indexBegin;
            canFitDigits = true;
        }
        if (!canFitDigits && _indexEnd + 1 == _stringEnd)
        {
            _indexEnd = _indexBegin + newSize - 1;
            _stringEnd = _indexEnd + 1;
            *_stringEnd = '\0';
            canFitDigits = true;
        }

        if (!canFitDigits)
        {
            // For simplicity only part of the string between indexEnd and stringEnd is copied
            // around
            size_t copyAmount = _stringEnd - _indexEnd;
            std::memmove(_indexBegin + newSize, _indexEnd + 1, copyAmount);

            _indexEnd = _indexBegin + newSize - 1;
            _stringEnd = _indexEnd + copyAmount;
        }
    }
}

} // namespace pf::util
