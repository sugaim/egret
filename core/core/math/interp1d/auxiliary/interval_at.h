#pragma once

#include <ranges>

namespace egret_detail::interp1d_impl {
// -----------------------------------------------------------------------------
//  [fn] interval_at
// -----------------------------------------------------------------------------
    template <std::ranges::forward_range Xs>
    constexpr auto interval_at(std::size_t i, Xs&& xs)
        -> std::pair<std::ranges::iterator_t<Xs>, std::ranges::iterator_t<Xs>>
    {
        auto xlit = std::ranges::next(std::ranges::begin(xs), static_cast<std::ranges::range_difference_t>(i));
        auto xrit = std::ranges::next(xlit);
        return {std::move(xlit), std::move(xrit)};
    }

} // namespace egret_detail::interp1d_impl
