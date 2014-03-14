//  Copyright Eric Niebler 2014
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//
#ifndef RANGES_V3_VIEW_ALL_HPP
#define RANGES_V3_VIEW_ALL_HPP

#include <utility>
#include <type_traits>
#include <range/v3/range_fwd.hpp>
#include <range/v3/size.hpp>
#include <range/v3/begin_end.hpp>
#include <range/v3/range_concepts.hpp>
#include <range/v3/range_concepts.hpp>
#include <range/v3/iterator_range.hpp>
#include <range/v3/utility/bindable.hpp>

namespace ranges
{
    inline namespace v3
    {
        namespace detail
        {
            // An Iterable is container-like if the constness of its reference type
            // is sensitive to the constness of the Iterable
            template<typename T>
            using owns_its_elements_t = not_t<
                std::is_same<
                    decltype(*ranges::begin(std::declval<as_ref_t<T>>())),
                    decltype(*ranges::begin(std::declval<as_cref_t<T>>()))>>;

            template<typename RandomAccessIterator, typename Size,
                CONCEPT_REQUIRES_(ranges::RandomAccessIterator<RandomAccessIterator>())>
            iterator_range<RandomAccessIterator>
            container_view_all2(RandomAccessIterator begin, RandomAccessIterator end, Size)
            {
                return {begin, end};
            }

            template<typename Iterator, typename Sentinel, typename Size>
            sized_iterator_range<Iterator, Sentinel>
            container_view_all2(Iterator begin, Sentinel end, Size size)
            {
                return {begin, end, size};
            }

            template<typename Iterator, typename Size>
            iterator_range<counted_iterator<Iterator>, counted_sentinel<Iterator>>
            container_view_all2(counted_iterator<Iterator> begin, counted_sentinel<Iterator> end, Size size)
            {
                RANGES_ASSERT(size == static_cast<Size>(end.count() - begin.count()));
                return {begin, end};
            }

            template<typename Iterator, typename Size>
            iterator_range<counted_iterator<Iterator>, counted_iterator<Iterator>>
            container_view_all2(counted_iterator<Iterator> begin, counted_iterator<Iterator> end, Size size)
            {
                RANGES_ASSERT(size == static_cast<Size>(end.count() - begin.count()));
                return {begin, end};
            }

            template<typename T>
            iterator_range<range_iterator_t<T>, range_sentinel_t<T>>
            container_view_all(T & t, concepts::Iterable)
            {
                return {ranges::begin(t), ranges::end(t)};
            };

            template<typename T>
            iterator_range<range_iterator_t<T>>
            container_view_all(T & t, concepts::Range)
            {
                return {ranges::begin(t), ranges::end(t)};
            };

            template<typename T>
            auto
            container_view_all(T & t, concepts::SizedIterable) ->
                decltype(detail::container_view_all2(ranges::begin(t), ranges::end(t), ranges::size(t)))
            {
                return detail::container_view_all2(ranges::begin(t), ranges::end(t), ranges::size(t));
            };

            template<typename T>
            auto
            container_view_all(T & t, concepts::SizedRange) ->
                decltype(detail::container_view_all2(ranges::begin(t), ranges::end(t), ranges::size(t)))
            {
                return detail::container_view_all2(ranges::begin(t), ranges::end(t), ranges::size(t));
            };
        }

        template<typename T, typename Enable = void>
        struct is_iterable
          : std::is_base_of<range_base, typename std::remove_reference<T>::type>
        {};

        namespace view
        {
            struct all_getter : bindable<all_getter>
            {
                /// If it's an iterable already, pass it though.
                template<typename T,
                    CONCEPT_REQUIRES_(Iterable<T>() &&
                                      is_iterable<T>::value)>
                static T invoke(all_getter, T && t)
                {
                    static_assert(!detail::owns_its_elements_t<T>::value,
                        "Ranges should have reference semantics.");
                    return std::forward<T>(t);
                }

                /// If it is container-like, turn it into an iterator_range
                template<typename T,
                    CONCEPT_REQUIRES_(Iterable<T>() &&
                                      !is_iterable<T>::value &&
                                      detail::owns_its_elements_t<T>::value)>
                static auto invoke(all_getter, T && t) ->
                    decltype(detail::container_view_all(t, range_concept_t<T>{}))
                {
                    static_assert(std::is_lvalue_reference<T>::value,
                        "Cannot get a view of a temporary container");
                    return detail::container_view_all(t, range_concept_t<T>{});
                }

                // TODO handle char const * by turning it into a delimited range
            };

            RANGES_CONSTEXPR all_getter all {};
        }

        template<typename ConvertibleToRange>
        using range_view_all_t = decltype(view::all(std::declval<ConvertibleToRange>()));
    }
}

#endif
