#include <ranges>
#include <limits>

#include <gtest/gtest.h>
#include <gtest/gtest-param-test.h>

#include <pf_utils/RangeAlgorithms.hpp>

// NOLINTNEXTLINE
TEST(RangeAlgorithms_ExponentialSearch, EmptyRange_ReturnsNullptr)
{
    std::ranges::empty_view<size_t> range;
    auto predicate = [](size_t /*i*/) { return true; };

    auto partitionPosition = pf::util::exponentialSearch(range, predicate);

    EXPECT_TRUE(partitionPosition == range.end());
}

struct SearchTestParameters
{
    size_t max;
    size_t partitionPoint;
};

// NOLINTNEXTLINE(readability-identifier-naming)
class RangeAlgorithms_ExponentialSearchParametrized
    : public ::testing::TestWithParam<SearchTestParameters>
{
};

// NOLINTNEXTLINE
TEST_P(RangeAlgorithms_ExponentialSearchParametrized, GivenPartitioned_ReturnsPartitionPosition)
{
    size_t max = GetParam().max;
    size_t actualPartitionPosition = GetParam().partitionPoint;
    auto range = std::ranges::views::iota(static_cast<size_t>(0), max);
    auto predicate = [actualPartitionPosition](size_t i) { return i < actualPartitionPosition; };

    auto partitionPosition = pf::util::exponentialSearch(range, predicate);

    EXPECT_EQ(partitionPosition, range.begin() + actualPartitionPosition);
}

size_t const MEDIUM_RANGE_MAX = 1000000;

// NOLINTNEXTLINE
INSTANTIATE_TEST_SUITE_P(
    RangeAlgorithms_ExponentialSearchMediumRange,
    RangeAlgorithms_ExponentialSearchParametrized,
    ::testing::Values(SearchTestParameters{MEDIUM_RANGE_MAX, 0},
                      SearchTestParameters{MEDIUM_RANGE_MAX, 1},
                      SearchTestParameters{MEDIUM_RANGE_MAX, MEDIUM_RANGE_MAX / 10 * 3},
                      SearchTestParameters{MEDIUM_RANGE_MAX, MEDIUM_RANGE_MAX / 2},
                      SearchTestParameters{MEDIUM_RANGE_MAX, MEDIUM_RANGE_MAX / 10 * 9},
                      SearchTestParameters{MEDIUM_RANGE_MAX, MEDIUM_RANGE_MAX - 1},
                      SearchTestParameters{MEDIUM_RANGE_MAX, MEDIUM_RANGE_MAX}));

size_t const LARGE_RANGE_MAX = std::numeric_limits<size_t>::max();

// NOLINTNEXTLINE
INSTANTIATE_TEST_SUITE_P(
    RangeAlgorithms_ExponentialSearchLargeRange,
    RangeAlgorithms_ExponentialSearchParametrized,
    ::testing::Values(SearchTestParameters{LARGE_RANGE_MAX, 0},
                      SearchTestParameters{LARGE_RANGE_MAX, 1},
                      SearchTestParameters{LARGE_RANGE_MAX, LARGE_RANGE_MAX / 10 * 3},
                      SearchTestParameters{LARGE_RANGE_MAX, LARGE_RANGE_MAX / 2},
                      SearchTestParameters{LARGE_RANGE_MAX, LARGE_RANGE_MAX / 10 * 9},
                      SearchTestParameters{LARGE_RANGE_MAX, LARGE_RANGE_MAX - 1},
                      SearchTestParameters{LARGE_RANGE_MAX, LARGE_RANGE_MAX}));
