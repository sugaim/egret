#pragma once

#include "core/utils/range_utils/find_interval.h"
#include "../concepts.h"

namespace egret_detail::interp1d_impl {
// -----------------------------------------------------------------------------
//  [fn] find_index_and_relpos
// -----------------------------------------------------------------------------
    template <std::ranges::forward_range Xs, typename X, typename Less = std::ranges::less>
    constexpr auto find_index_and_relpos(Xs&& xs, const X& x, Less less = {})
        -> std::pair<std::ranges::range_difference_t<Xs>, egret::math::interp1d::distance_result_t<std::ranges::range_reference_t<Xs>, X>>
    {
        namespace interp1d = egret::math::interp1d;
        const auto [xlit, xrit] = egret::util::find_interval(xs, x, less);
        const auto idx = std::ranges::distance(std::ranges::begin(xs), xlit);
        const auto w = interp1d::distance(*xlit, x) / interp1d::distance(*xlit, *xrit);
        return { idx, w };
    }

} // namespace egret_detail::interp1d_impl
