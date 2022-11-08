#pragma once

#include <concepts>
#include <iterator>
#include <ranges>
#include <nlohmann/json_fwd.hpp>
#include "concepts.h"
#include "../concepts.h"
#include "core/math/algebra/concepts.h"

namespace egret::math::interp1d {
// -----------------------------------------------------------------------------
//  [class] backward_difference
// -----------------------------------------------------------------------------
    class backward_difference {
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

            auto xprv = xit;
            if (++xit == xsn) {
                *oit = static_cast<value_type>(0);
                ++oit;
                return oit;
            }

            auto yprv = yit;
            ++yit;
            {
                // left end is calculated with forward difference
                auto xdist = interp1d::distance(*xprv, *xit);
                auto ydist = *yit - *yprv;
                *oit = static_cast<value_type>(std::move(ydist) / std::move(xdist));
                ++oit;
            }

            for (; xit != xsn; ++xprv, ++xit, ++yprv, ++yit, ++oit) {
                auto xdist = interp1d::distance(*xprv, *xit);
                auto ydist = *yit - *yprv;                
                *oit = static_cast<value_type>(std::move(ydist) / std::move(xdist));
            }
            return oit;
        }

    }; // class backward_difference

} // namespace egret::math::interp1d

namespace nlohmann {
    template <>
    struct adl_serializer<egret::math::interp1d::backward_difference> {
        template <typename Json>
        static void from_json(const Json& j, egret::math::interp1d::backward_difference&) noexcept {}

        template <typename Json>
        static void to_json(Json& j, const egret::math::interp1d::backward_difference&) noexcept {}
    };

} // namespace nlohmann
