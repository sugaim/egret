#pragma once

#include <ranges>
#include <optional>
#include <algorithm>
#include "core/assertions/exception.h"

namespace egret::util {
// -----------------------------------------------------------------------------
//  [fn] find_interval
// -----------------------------------------------------------------------------
    template <
        std::ranges::forward_range Xs, 
        typename X, 
        typename Proj = std::identity,
        std::indirect_strict_weak_order<
            const X*,
            std::projected<std::ranges::iterator_t<Xs>, Proj>
        > Less = std::ranges::less
    >
    constexpr auto find_interval(Xs&& xs, const X& x, Less less = {}, Proj&& proj = {})
        -> std::pair<std::ranges::iterator_t<Xs>, std::ranges::iterator_t<Xs>>
    {
        if constexpr (std::ranges::random_access_range<Xs>) {
            const auto beg = std::ranges::begin(xs);
            const auto end = std::ranges::end(xs);
            const auto sz = end - beg;
            if (sz < 2) {
                throw exception("Range must have 2 elements at least to use find_interval, but has only {}.", sz).record_stacktrace();
            }
            auto it = std::ranges::lower_bound(beg, end - 2, x, less, proj);
            if (it != beg && less(x, proj(*it))) {
                --it;
            }
            return std::pair {it, std::ranges::next(it)};
        }
        else {
            const auto end = std::ranges::end(xs);
            auto lit = std::ranges::begin(xs);
            if (lit == end) {
                throw exception("Range must have 2 elements at least to use find_interval, but has only 0.").record_stacktrace();
            }
            auto bound_checker = std::ranges::next(lit);
            if (bound_checker == end) {
                throw exception("Range must have 2 elements at least to use find_interval, but has only 1.").record_stacktrace();
            }
            if (!less(x, proj(*lit))) {
                ++bound_checker;
                for (; bound_checker != end; ++lit, ++bound_checker) {
                    if (!less(x, proj(*lit))) {
                        // lit <= x
                        break;
                    }
                }
            }
            return std::pair {lit, std::ranges::next(lit)};
        }
    }

} // namespace egret::util