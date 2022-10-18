#pragma once

#include <cmath>
#include <numbers>

namespace egret::math::stats {
// -----------------------------------------------------------------------------
//  [class] standard_normal_distribution
//  [value] standard_normal
// -----------------------------------------------------------------------------
    template <typename X>
    class standard_normal_distribution {
    public:
        constexpr X pdf(const X& x) const
            requires std::floating_point<X>
        {
            constexpr X overall = std::numbers::inv_sqrtpi_v<X> / std::numbers::sqrt2_v<X>;
            return std::exp(- 0.5 * x * x) * overall;
        }

        constexpr X cdf(const X& x) const
            requires std::floating_point<X>
        {
            return 0.5 * (1. + std::erf(x));
        }
    };
    
    template <typename X>
    inline constexpr auto stdnormal = standard_normal_distribution<X> {};

} // namespace egret::math::stats
