#pragma once

#include <ranges>
#include "core/assertions/assertion.h"

namespace egret_detail::interp1d_impl {
// -----------------------------------------------------------------------------
//  [fn] interpolatee_validation
// -----------------------------------------------------------------------------
    template <std::ranges::forward_range Xs, std::ranges::forward_range Ys, typename Less>
    void interpolatee_validation(const Xs& grids, const Ys& values, const Less& less, long min_size = 2)
    {
        const auto distx = std::ranges::distance(grids);
        const auto disty = std::ranges::distance(values);
        
        egret::assertion(std::cmp_equal(distx, disty), "The numbers of ranges must be the same. [grids.size={}, values.size={}]", distx, disty);
        egret::assertion(std::cmp_less_equal(min_size, distx), "Interpolatee must have at least {}, but is {}", min_size, distx);
        egret::assertion(std::ranges::is_sorted(grids, less), "Grid data of interpolatee must be sorted.");

        {
            const auto eq = [&less](const auto& lhs, const auto& rhs) { 
                return !std::invoke(less, lhs, rhs) && !std::invoke(less, rhs, lhs); 
            };
            const auto adjit = std::ranges::adjacent_find(grids, eq);
            egret::assertion(
                adjit == std::ranges::end(grids),
                "Duplicated grid values are found. [index={}]",
                std::ranges::distance(std::ranges::begin(grids), adjit)
            );
        }
    }

} // namespace egret_detail::interp1d_impl
