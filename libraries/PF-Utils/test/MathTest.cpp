#include <cstddef>
#include <random>
#include <string>
#include <limits>

#include <gtest/gtest.h>

#include <pf_utils/Math.hpp>

using UnsignedInt = unsigned int;
using SignedInt = int;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
std::uniform_int_distribution<SignedInt> signedIntsRandomRange(-10000, 10000);

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
std::uniform_int_distribution<UnsignedInt> unsignedIntsRandomRange(0, 10000);

size_t const RANDOM_TESTS_TRIALS_COUNT = 10;


// * digitsCountIterative for signed ints *

// NOLINTNEXTLINE
TEST(Math_DigitsCountIterativeSignedInt, GivenRandomNumber_ReturnsCorrectValue)
{
    std::random_device randomDevice;
    std::mt19937 rng(randomDevice());

    for (size_t trial = 0; trial < RANDOM_TESTS_TRIALS_COUNT; trial++)
    {
        SignedInt number = signedIntsRandomRange(rng);
        size_t actualDigitsCount = std::to_string(number).size() - (number < 0 ? 1 : 0);

        size_t digitsCount = pf::util::math::digitsCountIterative(number);

        EXPECT_EQ(digitsCount, actualDigitsCount);
    }
}

// NOLINTNEXTLINE
TEST(Math_DigitsCountIterativeSignedInt, GivenMaxNumber_ReturnsCorrectValue)
{
    SignedInt number = std::numeric_limits<SignedInt>::max();
    size_t actualDigitsCount = std::to_string(number).size();

    size_t digitsCount = pf::util::math::digitsCountIterative(number);

    EXPECT_EQ(digitsCount, actualDigitsCount);
}

// NOLINTNEXTLINE
TEST(Math_DigitsCountIterativeSignedInt, GivenMinNumber_ReturnsCorrectValue)
{
    SignedInt number = std::numeric_limits<SignedInt>::min();
    size_t actualDigitsCount = std::to_string(number).size() - (number < 0 ? 1 : 0);

    size_t digitsCount = pf::util::math::digitsCountIterative(number);

    EXPECT_EQ(digitsCount, actualDigitsCount);
}


// * digitsCountIterative for unsigned ints *

// NOLINTNEXTLINE
TEST(Math_DigitsCountIterativeUnsignedInt, GivenRandomNumber_ReturnsCorrectValue)
{
    std::random_device randomDevice;
    std::mt19937 rng(randomDevice());

    for (size_t trial = 0; trial < RANDOM_TESTS_TRIALS_COUNT; trial++)
    {
        UnsignedInt number = unsignedIntsRandomRange(rng);
        size_t actualDigitsCount = std::to_string(number).size() - (number < 0 ? 1 : 0);

        size_t digitsCount = pf::util::math::digitsCountIterative(number);

        EXPECT_EQ(digitsCount, actualDigitsCount);
    }
}

// NOLINTNEXTLINE
TEST(Math_DigitsCountIterativeUnsignedInt, GivenMaxNumber_ReturnsCorrectValue)
{
    UnsignedInt number = std::numeric_limits<UnsignedInt>::max();
    size_t actualDigitsCount = std::to_string(number).size();

    size_t digitsCount = pf::util::math::digitsCountIterative(number);

    EXPECT_EQ(digitsCount, actualDigitsCount);
}


// * digitsCount for signed ints *

// NOLINTNEXTLINE
TEST(Math_DigitsCountSignedInt, GivenZero_ReturnsCorrectValue)
{
    SignedInt number = 0;

    size_t digitsCount = pf::util::math::digitsCount(number);

    EXPECT_EQ(digitsCount, 1);
}

// NOLINTNEXTLINE
TEST(Math_DigitsCountSignedInt, GivenNine_ReturnsCorrectValue)
{
    SignedInt number = 9;

    size_t digitsCount = pf::util::math::digitsCount(number);

    EXPECT_EQ(digitsCount, 1);
}

// NOLINTNEXTLINE
TEST(Math_DigitsCountSignedInt, GivenTen_ReturnsCorrectValue)
{
    SignedInt number = 10;

    size_t digitsCount = pf::util::math::digitsCount(number);

    EXPECT_EQ(digitsCount, 2);
}

// NOLINTNEXTLINE
TEST(Math_DigitsCountSignedInt, GivenNegativeNumber_ReturnsCorrectValue)
{
    SignedInt number = -10;

    size_t digitsCount = pf::util::math::digitsCount(number);

    EXPECT_EQ(digitsCount, 2);
}

// NOLINTNEXTLINE
TEST(Math_DigitsCountSignedInt, GivenRandomNumber_ReturnsCorrectValue)
{
    std::random_device randomDevice;
    std::mt19937 rng(randomDevice());

    for (size_t trial = 0; trial < RANDOM_TESTS_TRIALS_COUNT; trial++)
    {
        SignedInt number = signedIntsRandomRange(rng);
        size_t actualDigitsCount = std::to_string(number).size() - (number < 0 ? 1 : 0);

        size_t digitsCount = pf::util::math::digitsCount(number);

        EXPECT_EQ(digitsCount, actualDigitsCount);
    }
}

// NOLINTNEXTLINE
TEST(Math_DigitsCountSignedInt, GivenMaxNumber_ReturnsCorrectValue)
{
    SignedInt number = std::numeric_limits<SignedInt>::max();
    size_t actualDigitsCount = std::to_string(number).size();

    size_t digitsCount = pf::util::math::digitsCount(number);

    EXPECT_EQ(digitsCount, actualDigitsCount);
}

// NOLINTNEXTLINE
TEST(Math_DigitsCountSignedInt, GivenMinNumber_ReturnsCorrectValue)
{
    SignedInt number = std::numeric_limits<SignedInt>::min();
    size_t actualDigitsCount = std::to_string(number).size() - (number < 0 ? 1 : 0);

    size_t digitsCount = pf::util::math::digitsCount(number);

    EXPECT_EQ(digitsCount, actualDigitsCount);
}


// * digitsCount for unsigned ints *

// NOLINTNEXTLINE
TEST(Math_DigitsCountUnsignedInt, GivenRandomNumber_ReturnsCorrectValue)
{
    std::random_device randomDevice;
    std::mt19937 rng(randomDevice());

    for (size_t trial = 0; trial < RANDOM_TESTS_TRIALS_COUNT; trial++)
    {
        UnsignedInt number = unsignedIntsRandomRange(rng);
        size_t actualDigitsCount = std::to_string(number).size() - (number < 0 ? 1 : 0);

        size_t digitsCount = pf::util::math::digitsCount(number);

        EXPECT_EQ(digitsCount, actualDigitsCount);
    }
}

// NOLINTNEXTLINE
TEST(Math_DigitsCountUnsignedInt, GivenMaxNumber_ReturnsCorrectValue)
{
    UnsignedInt number = std::numeric_limits<UnsignedInt>::max();
    size_t actualDigitsCount = std::to_string(number).size();

    size_t digitsCount = pf::util::math::digitsCount(number);

    EXPECT_EQ(digitsCount, actualDigitsCount);
}


// * isPoT *

// NOLINTNEXTLINE
TEST(Math_isPoT, GivenZero_ReturnsFalse)
{
    UnsignedInt number = 0;

    bool isPoT = pf::util::math::isPoT(number);

    EXPECT_EQ(isPoT, false);
}

// NOLINTNEXTLINE
TEST(Math_isPoT, GivenOne_ReturnsTrue)
{
    UnsignedInt number = 1;

    bool isPoT = pf::util::math::isPoT(number);

    EXPECT_EQ(isPoT, true);
}

// NOLINTNEXTLINE
TEST(Math_isPoT, GivenSignedMinNumber_ReturnsTrue)
{
    SignedInt number = std::numeric_limits<SignedInt>::min();

    bool isPoT = pf::util::math::isPoT(number);

    EXPECT_EQ(isPoT, true);
}

// NOLINTNEXTLINE
TEST(Math_isPoT, GivenPowersOfTwo_ReturnsTrue)
{
    for (size_t i = 0; i < 10; i++)
    {
        UnsignedInt potNumber = pf::util::math::pow(2, i);

        bool isPoT = pf::util::math::isPoT(potNumber);

        EXPECT_EQ(isPoT, true);
    }
}

// NOLINTNEXTLINE
TEST(Math_isPoT, GivenNegativePowersOfTwo_ReturnsTrue)
{
    for (size_t i = 0; i < 10; i++)
    {
        SignedInt potNumber = -pf::util::math::pow(2, i);

        bool isPoT = pf::util::math::isPoT(potNumber);

        EXPECT_EQ(isPoT, true);
    }
}

// NOLINTNEXTLINE
TEST(Math_isPoT, GivenNonPowersOfTwo_ReturnsFalse)
{
    for (size_t i = 2; i <= 10; i++)
    {
        UnsignedInt potNumber = pf::util::math::pow(2, i) - 1;

        bool isPoT = pf::util::math::isPoT(potNumber);

        EXPECT_EQ(isPoT, false);
    }
}

// NOLINTNEXTLINE
TEST(Math_isPoT, GivenNegativeNonPowersOfTwo_ReturnsTrue)
{
    for (size_t i = 2; i <= 10; i++)
    {
        SignedInt potNumber = -pf::util::math::pow(2, i) + 1;

        bool isPoT = pf::util::math::isPoT(potNumber);

        EXPECT_EQ(isPoT, false);
    }
}
