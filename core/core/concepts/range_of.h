#pragma once

#include <ranges>
#include <type_traits>

namespace egret::cpt {
// -----------------------------------------------------------------------------
//  [concept] range_of
// -----------------------------------------------------------------------------
    template <typename R, typename E>
    concept range_of = std::ranges::range<R> &&
        std::is_convertible_v<std::ranges::range_reference_t<R>, E>;

// -----------------------------------------------------------------------------
//  [concept] input_range_of
// -----------------------------------------------------------------------------
    template <typename R, typename E>
    concept input_range_of = std::ranges::input_range<R> &&
        std::is_convertible_v<std::ranges::range_reference_t<R>, E>;

// -----------------------------------------------------------------------------
//  [concept] forward_range_of
// -----------------------------------------------------------------------------
    template <typename R, typename E>
    concept forward_range_of = std::ranges::forward_range<R> &&
        std::is_convertible_v<std::ranges::range_reference_t<R>, E>;

// -----------------------------------------------------------------------------
//  [concept] bidirectional_range_of
// -----------------------------------------------------------------------------
    template <typename R, typename E>
    concept bidirectional_range_of = std::ranges::bidirectional_range<R> &&
        std::is_convertible_v<std::ranges::range_reference_t<R>, E>;

// -----------------------------------------------------------------------------
//  [concept] random_access_range_of
// -----------------------------------------------------------------------------
    template <typename R, typename E>
    concept random_access_range_of = std::ranges::random_access_range<R> &&
        std::is_convertible_v<std::ranges::range_reference_t<R>, E>;

// -----------------------------------------------------------------------------
//  [concept] contiguous_range_of
// -----------------------------------------------------------------------------
    template <typename R, typename E>
    concept contiguous_range_of = std::ranges::contiguous_range<R> &&
        std::is_convertible_v<std::ranges::range_reference_t<R>, E>;

} // namespace egret::cpt
