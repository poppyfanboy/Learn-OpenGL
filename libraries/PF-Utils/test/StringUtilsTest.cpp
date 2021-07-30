/**
 * I just wanted to get a hang of writing simple tests / figure out which naming convention to adopt
 * / setup tests GUI in my editor. These tests are obviously useless.
 */

#include <string>
#include <cctype>
#include <vector>
#include <random>
#include <algorithm>
#include <cstddef>

#include <gtest/gtest.h>

#include <pf_utils/StringUtils.hpp>

// * padRight tests *

// NOLINTNEXTLINE
TEST(StringUtils_PadRight, EmptyString_ReturnsCorrectValue)
{
    std::string originalString;
    size_t paddedStringSize = 5;

    std::string paddedString = pf::util::string::padRight(originalString, paddedStringSize);

    EXPECT_EQ(paddedString, std::string(paddedStringSize, ' '));
}

// NOLINTNEXTLINE
TEST(StringUtils_PadRight, EmptyStringWithEmptyPadding_ReturnsEmptyString)
{
    std::string originalString;
    size_t paddedStringSize = 0;

    std::string paddedString = pf::util::string::padRight(originalString, paddedStringSize);

    EXPECT_EQ(paddedString, "");
}

// NOLINTNEXTLINE
TEST(StringUtils_PadRight, PaddingShorterThanString_ReturnOriginalString)
{
    std::string originalString = "long-string";
    size_t paddedStringSize = 4;

    std::string paddedString = pf::util::string::padRight(originalString, paddedStringSize);

    EXPECT_EQ(paddedString, originalString);
}

// NOLINTNEXTLINE
TEST(StringUtils_PadRight, PaddingLongerThanString_ReturnPaddedString)
{
    std::string originalString = "12345678";
    size_t paddedStringSize = 10;

    std::string paddedString = pf::util::string::padRight(originalString, paddedStringSize);

    EXPECT_EQ(paddedString, "12345678  ");
}

// NOLINTNEXTLINE
TEST(StringUtils_PadRight, CustomPaddingCharacter_ReturnPaddedString)
{
    std::string originalString = "12345678";
    size_t paddedStringSize = 10;
    char paddingChar = '_';

    std::string paddedString =
        pf::util::string::padRight(originalString, paddedStringSize, paddingChar);

    EXPECT_EQ(paddedString, "12345678__");
}


// * padLeft tests *

// NOLINTNEXTLINE
TEST(StringUtils_PadLeft, EmptyString_ReturnsCorrectValue)
{
    std::string originalString;
    size_t paddedStringSize = 5;

    std::string paddedString = pf::util::string::padLeft(originalString, paddedStringSize);

    EXPECT_EQ(paddedString, std::string(paddedStringSize, ' '));
}

// NOLINTNEXTLINE
TEST(StringUtils_PadLeft, EmptyStringWithEmptyPadding_ReturnsEmptyString)
{
    std::string originalString;
    size_t paddedStringSize = 0;

    std::string paddedString = pf::util::string::padLeft(originalString, paddedStringSize);

    EXPECT_EQ(paddedString, "");
}

// NOLINTNEXTLINE
TEST(StringUtils_PadLeft, PaddingShorterThanString_ReturnOriginalString)
{
    size_t paddedStringSize = 4;
    std::string originalString = "long-string";

    std::string paddedString = pf::util::string::padLeft(originalString, paddedStringSize);

    EXPECT_EQ(paddedString, originalString);
}

// NOLINTNEXTLINE
TEST(StringUtils_PadLeft, PaddingLongerThanString_ReturnPaddedString)
{
    size_t paddedStringSize = 10;
    std::string originalString = "12345678";

    std::string paddedString = pf::util::string::padLeft(originalString, paddedStringSize);

    EXPECT_EQ(paddedString, "  12345678");
}

// NOLINTNEXTLINE
TEST(StringUtils_PadLeft, CustomPaddingCharacter_ReturnPaddedString)
{
    std::string originalString = "12345678";
    size_t paddedStringSize = 10;
    char paddingCharacter = '_';

    std::string paddedString =
        pf::util::string::padLeft(originalString, paddedStringSize, paddingCharacter);

    EXPECT_EQ(paddedString, "__12345678");
}


// * padLeftWithZeros tests *

// NOLINTNEXTLINE
TEST(StringUtils_PadLeftWithZeros, PaddingShorterThanNumber_ReturnOriginalNumber)
{
    size_t paddedStringSize = 3;
    size_t number = 12345;

    std::string paddedNumber = pf::util::string::padLeftWithZeros(number, paddedStringSize);

    EXPECT_EQ(paddedNumber, std::to_string(number));
}


// NOLINTNEXTLINE
TEST(StringUtils_PadLeftWithZeros, PaddingLongerThanNumber_ReturnPaddedNumber)
{
    size_t paddedStringSize = 5;
    size_t number = 123;

    std::string paddedNumber = pf::util::string::padLeftWithZeros(number, paddedStringSize);

    EXPECT_EQ(paddedNumber, "00123");
}


// * random strings tests *

// NOLINTNEXTLINE
TEST(StringUtils_RandomString, UserDefinedStringSize_ReturnStringWithCorrectSize)
{
    size_t stringSize = 10;
    std::vector<char> charsToBuildStringFrom{'a', 'b', '1', '@', '#'};
    std::random_device randomDevice;
    std::mt19937 rng(randomDevice());

    std::string randomString = pf::util::string::random(stringSize, rng, charsToBuildStringFrom);

    EXPECT_EQ(randomString.size(), stringSize);
}

bool stringConsistsOfCharacters(std::string const &string, std::vector<char> const &characters)
{
    return std::ranges::all_of(
        string.cbegin(),
        string.cend(),
        [characters](char c)
        { return std::find(characters.begin(), characters.end(), c) != characters.end(); });
}

// NOLINTNEXTLINE
TEST(StringUtils_RandomString, UserDefinedCharactersAndRNG_ReturnStringWithCorrectCharacters)
{
    size_t stringSize = 10;
    std::vector<char> charsToBuildStringFrom{'a', 'b', '1', '@', '#'};
    std::random_device randomDevice;
    std::mt19937 rng(randomDevice());

    std::string randomString = pf::util::string::random(stringSize, rng, charsToBuildStringFrom);

    EXPECT_TRUE(stringConsistsOfCharacters(randomString, charsToBuildStringFrom));
}


// * random alphanumeric strings tests *

// NOLINTNEXTLINE(readability-identifier-naming)
class StringUtils_RandomAlphanumeric : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {

        for (char c = '0'; c <= '9'; c++)
        {
            alphanumericChars.push_back(c);
        }
        for (char c = 'a'; c <= 'z'; c++)
        {
            alphanumericChars.push_back(c);
            alphanumericChars.push_back(static_cast<char>(std::toupper(c)));
        }
    }

    static bool isAlphanumeric(std::string const &string)
    {
        return stringConsistsOfCharacters(string, alphanumericChars);
    }

    // NOLINTNEXTLINE
    static std::vector<char> alphanumericChars;
};

// NOLINTNEXTLINE
std::vector<char> StringUtils_RandomAlphanumeric::alphanumericChars;

// NOLINTNEXTLINE
TEST_F(StringUtils_RandomAlphanumeric, UserDefinedStringSize_ReturnStringWithCorrectSize)
{

    size_t const stringSize = 10;
    std::random_device randomDevice;
    std::mt19937 rng(randomDevice());

    std::string randomString = pf::util::string::randomAlphanumeric(stringSize, rng);

    EXPECT_EQ(randomString.size(), stringSize);
}

// NOLINTNEXTLINE
TEST_F(StringUtils_RandomAlphanumeric, UserDefinedRNG_ReturnAlphanumericString)
{
    size_t const stringSize = 10;
    std::random_device randomDevice;
    std::mt19937 rng(randomDevice());

    std::string randomString = pf::util::string::randomAlphanumeric(stringSize, rng);

    EXPECT_TRUE(isAlphanumeric(randomString));
}

// NOLINTNEXTLINE
TEST_F(StringUtils_RandomAlphanumeric, UnspecifiedRNG_ReturnAlphanumericString)
{
    size_t const stringSize = 10;

    std::string randomString = pf::util::string::randomAlphanumeric(stringSize);

    EXPECT_TRUE(isAlphanumeric(randomString));
}
