#pragma once

#include "core/utils/range_utils/find_interval.h"
#include "../concepts.h"
#include "relpos.h"

namespace egret_detail::interp1d_impl {
// -----------------------------------------------------------------------------
//  [fn] find_index_and_relpos
// -----------------------------------------------------------------------------
    template <
        std::ranges::forward_range Xs, 
        egret::math::interp1d::relpos_computable_from<std::ranges::range_reference_t<Xs>> X,
        std::strict_weak_order<std::ranges::range_reference_t<Xs>, const X> Less
    >
    constexpr auto find_index_and_relpos(Xs&& xs, const X& x, const Less& less)
        -> std::pair<std::ranges::range_difference_t<Xs>, egret::math::interp1d::relpos_t<X, std::ranges::range_reference_t<Xs>>>
    {
        namespace interp1d = egret::math::interp1d;
        const auto [xlit, xrit] = egret::util::find_interval(xs, x, less);
        const auto idx = std::ranges::distance(std::ranges::begin(xs), xlit);
        return { idx, relpos(x).between(*xlit, *xrit) };
    }

    template <
        typename RelposType,
        std::ranges::forward_range Xs, 
        egret::math::interp1d::relpos_computable_from<std::ranges::range_reference_t<Xs>> X,
        std::strict_weak_order<std::ranges::range_reference_t<Xs>, const X> Less
    >
        requires std::convertible_to<egret::math::interp1d::relpos_t<X, std::ranges::range_reference_t<Xs>>, RelposType>
    constexpr auto find_index_and_relpos_as(Xs&& xs, const X& x, const Less& less)
        -> std::pair<std::ranges::range_difference_t<Xs>, RelposType>
    {
        namespace interp1d = egret::math::interp1d;
        const auto [xlit, xrit] = egret::util::find_interval(xs, x, less);
        const auto idx = std::ranges::distance(std::ranges::begin(xs), xlit);
        return { idx, static_cast<RelposType>(relpos(x).between(*xlit, *xrit)) };
    }
    
} // namespace egret_detail::interp1d_impl
