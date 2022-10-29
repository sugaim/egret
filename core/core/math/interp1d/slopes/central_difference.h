#pragma once

#include <concepts>
#include <iterator>
#include <ranges>
#include "concepts.h"
#include "../concepts.h"
#include "core/math/algebra/concepts.h"

namespace egret::math::interp1d {
// -----------------------------------------------------------------------------
//  [class] central_difference
// -----------------------------------------------------------------------------
    class central_difference {
    public:
        template <
            std::forward_iterator XIt, std::sentinel_for<XIt> XSn, 
            std::forward_iterator YIt, 
            std::output_iterator<std::iter_value_t<YIt>> OIt
        >
            requires cpt::subtractible<std::iter_reference_t<YIt>>
        constexpr OIt generate(XIt xit, const XSn& xsn, YIt yit, OIt oit) const
        {
            using value_type = std::iter_value_t<YIt>;

            if (xit == xsn) {
                return oit;
            }

            auto xnit = std::ranges::next(xit);
            if (xnit == xsn) {
                *oit = static_cast<value_type>(0);
                ++oit;
                return oit;
            }

            auto ynit = std::next(yit);
            {
                // left end is calculated with forward difference
                auto xdist = interp1d::distance(*xit, *xnit);
                auto ydist = *ynit - *yit;
                *oit = static_cast<value_type>(std::move(ydist) / std::move(xdist));
                ++xnit; ++ynit; // advance iterators to make xnit/ynit next-next of xit/yit.
                ++oit;
            }

            for (; xnit != xsn; ++xit, ++xnit, ++yit, ++ynit, ++oit) {
                auto xdist = interp1d::distance(*xit, *xnit);
                auto ydist = *ynit - *yit;                
                *oit = static_cast<value_type>(std::move(ydist) / std::move(xdist));
            }

            {
                // right end is calculated with backward difference
                const auto xbck = std::ranges::next(xit);
                const auto ybck = std::ranges::next(yit);
                auto xdist = interp1d::distance(*xit, *xbck);
                auto ydist = *ybck - *yit;
                *oit = static_cast<value_type>(std::move(ydist) / std::move(xdist));
                ++oit;
            }
            return oit;
        }

    }; // class central_difference

} // namespace egret::math::interp1d