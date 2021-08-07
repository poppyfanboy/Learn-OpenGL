#ifndef RANGE_ALGORITHMS_HPP
#define RANGE_ALGORITHMS_HPP

#include <iterator>
#include <ranges>
#include <concepts>
#include <algorithm>
#include <limits>
#include <functional>

namespace pf::util
{

template <std::forward_iterator Iterator,
          std::sentinel_for<Iterator> Sentinel,
          typename Projection = std::identity,
          std::indirect_unary_predicate<std::projected<Iterator, Projection>> Predicate>
requires(std::is_integral_v<std::iter_difference_t<Iterator>>)
Iterator
exponentialSearch(Iterator first, Sentinel last, Predicate predicate, Projection projection = {});

template <std::ranges::forward_range Range,
          typename Projection = std::identity,
          std::indirect_unary_predicate<std::projected<std::ranges::iterator_t<Range>, Projection>>
              Predicate>
std::ranges::borrowed_iterator_t<Range>
exponentialSearch(Range &&range, Predicate predicate, Projection projection = {});


// * Templates definitions *

template <std::forward_iterator Iterator,
          std::sentinel_for<Iterator> Sentinel,
          typename Projection,
          std::indirect_unary_predicate<std::projected<Iterator, Projection>> Predicate>
requires(std::is_integral_v<std::iter_difference_t<Iterator>>)
Iterator
exponentialSearch(Iterator first, Sentinel last, Predicate predicate, Projection projection)
{
    using IteratorDifference = std::iter_difference_t<Iterator>;
    using Subrange = std::ranges::subrange<Iterator, Sentinel>;

    auto step = IteratorDifference(2);
    auto left = first;
    auto right = first;

    while (true)
    {
        auto distanceToSentinel = std::ranges::distance(right, last);
        if (distanceToSentinel <= 0)
        {
            return std::ranges::partition_point(
                Subrange(left, last), predicate, projection);
        }
        if (!std::invoke(predicate, std::invoke(projection, *right)))
        {
            return std::ranges::partition_point(
                Subrange(left, std::next(right)), predicate, projection);
        }

        left = std::next(right);
        if (step <= std::numeric_limits<IteratorDifference>::max() / 2)
        {
            std::advance(right, step);
            step *= 2;
        }
        else
        {
            right = std::next(first, std::numeric_limits<IteratorDifference>::max() - 1);
        }
    }
    return last;
}

template <std::ranges::forward_range Range,
          typename Projection,
          std::indirect_unary_predicate<std::projected<std::ranges::iterator_t<Range>, Projection>>
              Predicate>
std::ranges::borrowed_iterator_t<Range>
exponentialSearch(Range &&range, Predicate predicate, Projection projection)
{
    return exponentialSearch(std::ranges::begin(range),
                             std::ranges::end(range),
                             std::move(predicate),
                             std::move(projection));
}

} // namespace pf::util

#endif // !RANGE_ALGORITHMS_HPP
