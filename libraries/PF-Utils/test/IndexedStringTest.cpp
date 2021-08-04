#include <cstddef>

#include <gtest/gtest.h>

#include <pf_utils/IndexedString.hpp>

size_t const STRESS_TEST_ITERATIONS_COUNT = 10;

// * withIndex, index is at the end *

// NOLINTNEXTLINE
TEST(IndexedString_WithIndexAtTheEnd, DigitsCountSameAsIndexSize_FillsWholeSpace)
{
    pf::util::IndexedString indexedString("Test?????");

    indexedString.withIndex(12345);

    EXPECT_STREQ(indexedString.data(), "Test12345");
}

// NOLINTNEXTLINE
TEST(IndexedString_WithIndexAtTheEnd, DigitsCountSmallerThanIndexSize_NullCharIsInsertedEarlier)
{
    pf::util::IndexedString indexedString("Test?????");

    indexedString.withIndex(123);

    EXPECT_STREQ(indexedString.data(), "Test123");
}

// NOLINTNEXTLINE
TEST(IndexedString_WithIndexAtTheEnd, StressTest_ReturnsCorrectStrings)
{
    pf::util::IndexedString indexedString("Test????????");

    for (size_t i = 0; i < STRESS_TEST_ITERATIONS_COUNT; i++)
    {
        indexedString.withIndex(1);
        EXPECT_STREQ(indexedString.data(), "Test1");

        indexedString.withIndex(23456);
        EXPECT_STREQ(indexedString.data(), "Test23456");

        indexedString.withIndex(78);
        EXPECT_STREQ(indexedString.data(), "Test78");

        indexedString.withIndex(12345678);
        EXPECT_STREQ(indexedString.data(), "Test12345678");

        indexedString.withIndex(9);
        EXPECT_STREQ(indexedString.data(), "Test9");
    }
}

// NOLINTNEXTLINE
TEST(IndexedString_WithIndexAtTheEnd, StringViewStressTest_ReturnsCorrectStrings)
{
    pf::util::IndexedString indexedString("Test????????");

    for (size_t i = 0; i < STRESS_TEST_ITERATIONS_COUNT; i++)
    {
        indexedString.withIndex(1);
        EXPECT_STREQ(indexedString.data(), "Test1");

        indexedString.withIndex(23456);
        EXPECT_STREQ(indexedString.data(), "Test23456");

        indexedString.withIndex(78);
        EXPECT_STREQ(indexedString.data(), "Test78");

        indexedString.withIndex(12345678);
        EXPECT_STREQ(indexedString.data(), "Test12345678");

        indexedString.withIndex(9);
        EXPECT_STREQ(indexedString.data(), "Test9");
    }
}

// * withIndex, index is in the middle *

// NOLINTNEXTLINE
TEST(IndexedString_WithIndexInTheMiddle, DigitsCountSameAsIndexSize_FillsWholeSpace)
{
    pf::util::IndexedString indexedString("Test?????Test");

    indexedString.withIndex(12345);

    EXPECT_STREQ(indexedString.data(), "Test12345Test");
}

TEST(IndexedString_WithIndexInTheMiddle,                             // NOLINT
     DigitsCountSmallerThanIndexSize_RemainingSpaceFilledWithFiller) // NOLINT
{
    pf::util::IndexedString indexedString("Test?????Test");

    indexedString.withIndex(123);

    EXPECT_STREQ(indexedString.data(), "Test123Test");
}

// NOLINTNEXTLINE
TEST(IndexedString_WithIndexInTheMiddle, StressTest_ReturnsCorrectStrings)
{
    pf::util::IndexedString indexedString("Test????????Test");

    for (size_t i = 0; i < STRESS_TEST_ITERATIONS_COUNT; i++)
    {
        indexedString.withIndex(1);
        EXPECT_STREQ(indexedString.data(), "Test1Test");

        indexedString.withIndex(23456);
        EXPECT_STREQ(indexedString.data(), "Test23456Test");

        indexedString.withIndex(78);
        EXPECT_STREQ(indexedString.data(), "Test78Test");

        indexedString.withIndex(12345678);
        EXPECT_STREQ(indexedString.data(), "Test12345678Test");

        indexedString.withIndex(9);
        EXPECT_STREQ(indexedString.data(), "Test9Test");
    }
}

// NOLINTNEXTLINE
TEST(IndexedString_WithIndexInTheMiddle, StringViewStressTest_ReturnsCorrectStrings)
{
    pf::util::IndexedString indexedString("Test????????Test");

    for (size_t i = 0; i < STRESS_TEST_ITERATIONS_COUNT; i++)
    {
        indexedString.withIndex("A");
        EXPECT_STREQ(indexedString.data(), "TestATest");

        indexedString.withIndex("CDEFG");
        EXPECT_STREQ(indexedString.data(), "TestCDEFGTest");

        indexedString.withIndex("HI");
        EXPECT_STREQ(indexedString.data(), "TestHITest");

        indexedString.withIndex("QWERTYUI");
        EXPECT_STREQ(indexedString.data(), "TestQWERTYUITest");

        indexedString.withIndex("S");
        EXPECT_STREQ(indexedString.data(), "TestSTest");
    }
}


// * withIndex, index is in the begining *

// NOLINTNEXTLINE
TEST(IndexedString_WithIndexInTheBegining, DigitsCountSameAsIndexSize_FillsWholeSpace)
{
    pf::util::IndexedString indexedString("?????Test");

    indexedString.withIndex(12345);

    EXPECT_STREQ(indexedString.data(), "12345Test");
}

// NOLINTNEXTLINE
TEST(IndexedString_WithIndexInTheBegining, DigitsCountSmallerThanIndexSize_StringBeginingIsShifted)
{
    pf::util::IndexedString indexedString("?????Test");

    indexedString.withIndex(123);

    EXPECT_STREQ(indexedString.data(), "123Test");
}

// NOLINTNEXTLINE
TEST(IndexedString_WithIndexInTheBegining, StressTest_ReturnsCorrectStrings)
{
    pf::util::IndexedString indexedString("????????Test");

    for (size_t i = 0; i < STRESS_TEST_ITERATIONS_COUNT; i++)
    {
        indexedString.withIndex(1);
        EXPECT_STREQ(indexedString.data(), "1Test");

        indexedString.withIndex(23456);
        EXPECT_STREQ(indexedString.data(), "23456Test");

        indexedString.withIndex(78);
        EXPECT_STREQ(indexedString.data(), "78Test");

        indexedString.withIndex(12345678);
        EXPECT_STREQ(indexedString.data(), "12345678Test");

        indexedString.withIndex(9);
        EXPECT_STREQ(indexedString.data(), "9Test");
    }
}


// * withIndex, whole string is an index *

// NOLINTNEXTLINE
TEST(IndexedString_WholeStringIsIndex, DigitsCountSameAsIndexSize_FillsWholeSpace)
{
    pf::util::IndexedString indexedString("?????");

    indexedString.withIndex(12345);

    EXPECT_STREQ(indexedString.data(), "12345");
}

// NOLINTNEXTLINE
TEST(IndexedString_WholeStringIsIndex, DigitsCountSmallerThanIndexSize_StringIsShortened)
{
    pf::util::IndexedString indexedString("?????");

    indexedString.withIndex(123);

    EXPECT_STREQ(indexedString.data(), "123");
}

// NOLINTNEXTLINE
TEST(IndexedString_WholeStringIsIndex, StressTest_ReturnsCorrectStrings)
{
    pf::util::IndexedString indexedString("????????");

    for (size_t i = 0; i < STRESS_TEST_ITERATIONS_COUNT; i++)
    {
        indexedString.withIndex(1);
        EXPECT_STREQ(indexedString.data(), "1");

        indexedString.withIndex(23456);
        EXPECT_STREQ(indexedString.data(), "23456");

        indexedString.withIndex(78);
        EXPECT_STREQ(indexedString.data(), "78");

        indexedString.withIndex(12345678);
        EXPECT_STREQ(indexedString.data(), "12345678");

        indexedString.withIndex(9);
        EXPECT_STREQ(indexedString.data(), "9");
    }
}


// * withPaddedIndex, index is in the middle *

// NOLINTNEXTLINE
TEST(IndexedString_WithPaddedIndexInTheMiddle, DigitsCountSameAsIndexSize_FillsWholeSpace)
{
    pf::util::IndexedString indexedString("Test?????Test");

    indexedString.withPaddedIndex(12345);

    EXPECT_STREQ(indexedString.data(), "Test12345Test");
}

TEST(IndexedString_WithPaddedIndexInTheMiddle,                       // NOLINT
     DigitsCountSmallerThanIndexSize_RemainingSpaceFilledWithFiller) // NOLINT
{
    pf::util::IndexedString indexedString("Test?????Test");

    indexedString.withPaddedIndex(123);

    EXPECT_STREQ(indexedString.data(), "Test00123Test");
}

// NOLINTNEXTLINE
TEST(IndexedString_WithPaddedIndexInTheMiddle, StressTest_ReturnsCorrectStrings)
{
    pf::util::IndexedString indexedString("Test????????Test");

    for (size_t i = 0; i < STRESS_TEST_ITERATIONS_COUNT; i++)
    {
        indexedString.withPaddedIndex(1);
        EXPECT_STREQ(indexedString.data(), "Test00000001Test");

        indexedString.withPaddedIndex(23456);
        EXPECT_STREQ(indexedString.data(), "Test00023456Test");

        indexedString.withPaddedIndex(78);
        EXPECT_STREQ(indexedString.data(), "Test00000078Test");

        indexedString.withPaddedIndex(12345678);
        EXPECT_STREQ(indexedString.data(), "Test12345678Test");

        indexedString.withPaddedIndex(9);
        EXPECT_STREQ(indexedString.data(), "Test00000009Test");
    }
}

// NOLINTNEXTLINE
TEST(IndexedString_WithPaddedIndexInTheMiddle, StringViewStressTest_ReturnsCorrectStrings)
{
    pf::util::IndexedString indexedString("Test????????Test");
    indexedString.filler(' ');

    indexedString.withPaddedIndex("A");
    EXPECT_STREQ(indexedString.data(), "Test       ATest");

    indexedString.withPaddedIndex("CDEFG");
    EXPECT_STREQ(indexedString.data(), "Test   CDEFGTest");

    indexedString.withPaddedIndex("HI");
    EXPECT_STREQ(indexedString.data(), "Test      HITest");

    indexedString.withPaddedIndex("QWERTYUI");
    EXPECT_STREQ(indexedString.data(), "TestQWERTYUITest");

    indexedString.withPaddedIndex("S");
    EXPECT_STREQ(indexedString.data(), "Test       STest");
}
