#pragma once

#include <concepts>
#include <iterator>
#include <ranges>
#include "concepts.h"
#include "../concepts.h"
#include "core/math/algebra/concepts.h"

namespace egret::math::interp1d {
// -----------------------------------------------------------------------------
//  [class] forward_difference
// -----------------------------------------------------------------------------
    class forward_difference {
    private:
        template <
            std::bidirectional_iterator XIt, std::sentinel_for<XIt> XSn, 
            std::bidirectional_iterator YIt, 
            std::output_iterator<std::iter_value_t<YIt>> OIt
        >
            requires cpt::subtractible<std::iter_reference_t<YIt>>
        constexpr OIt generate_impl(XIt xit, const XSn& xsn, YIt yit, OIt oit) const
        {
            using value_type = std::iter_value_t<YIt>;

            if (xit == xsn) {
                return oit;
            }

            auto xnit = std::next(xit);
            if (xnit == xsn) {
                *oit = static_cast<value_type>(0);
                ++oit;
                return oit;
            }

            auto ynit = std::next(yit);
            for (; xnit != xsn; ++xit, ++xnit, ++yit, ++ynit, ++oit) {
                auto xdist = interp1d::distance(*xit, *xnit);
                auto ydist = *ynit - *yit;                
                *oit = static_cast<value_type>(std::move(ydist) / std::move(xdist));
            }

            {
                // right end is calculated with backward difference
                const auto xprv = std::ranges::prev(xit);
                const auto yprv = std::ranges::prev(yit);
                auto xdist = interp1d::distance(*xprv, *xit);
                auto ydist = *yit - *yprv;
                *oit = static_cast<value_type>(std::move(ydist) / std::move(xdist));
                ++oit;
            }            
            return oit;
        }

        template <
            std::forward_iterator XIt, std::sentinel_for<XIt> XSn, 
            std::forward_iterator YIt, 
            std::output_iterator<std::iter_value_t<YIt>> OIt
        >
            requires cpt::subtractible<std::iter_reference_t<YIt>>
        constexpr OIt generate_impl(XIt xit, const XSn& xsn, YIt yit, OIt oit) const
        {
            using value_type = std::iter_value_t<YIt>;

            if (xit == xsn) {
                return oit;
            }

            auto xnit = std::next(xit);
            if (xnit == xsn) {
                *oit = static_cast<value_type>(0);
                return oit;
            }

            // these will be used for compute backward difference at the last interval
            auto xprv = xit;
            auto yprv = yit;
            std::int_fast32_t counter = 0;

            auto ynit = std::next(yit);
            for (; xnit != xsn; ++xit, ++xnit, ++yit, ++ynit, ++oit, ++counter) {
                auto xdist = interp1d::distance(*xit, *xnit);
                auto ydist = *ynit - *yit;
                *oit = static_cast<value_type>(std::move(ydist) / std::move(xdist));
            }

            {
                // right end is calculated with backward difference
                std::ranges::advance(xprv, static_cast<std::iter_difference_t<XIt>>(counter - 1));
                std::ranges::advance(yprv, static_cast<std::iter_difference_t<YIt>>(counter - 1));
                auto xdist = interp1d::distance(*xprv, *xit);
                auto ydist = *yit - *yprv;
                *oit = static_cast<value_type>(std::move(ydist) / std::move(xdist));
                ++oit;
            }            
            return oit;
        }
        
    public:
        template <
            std::forward_iterator XIt, std::sentinel_for<XIt> XSn, 
            std::forward_iterator YIt, 
            std::output_iterator<std::iter_value_t<YIt>> OIt
        >
            requires cpt::subtractible<std::iter_reference_t<YIt>>
        constexpr OIt generate(XIt xit, const XSn& xsn, YIt yit, OIt oit) const
        {
            return this->generate_impl(std::move(xit), xsn, std::move(yit), std::move(oit));
        }

    }; // class forward_difference

} // namespace egret::math::interp1d

namespace nlohmann {
    template <>
    struct adl_serializer<egret::math::interp1d::forward_difference> {
        template <typename Json>
        static void from_json(const Json& j, egret::math::interp1d::forward_difference&) noexcept {}

        template <typename Json>
        static void to_json(Json& j, const egret::math::interp1d::forward_difference&) noexcept {}
    };

} // namespace nlohmann
