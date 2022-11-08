#pragma once

#include <chrono>
#include "concepts.h"

namespace egret_detail::df_impl {
    void discount_factor(auto&&, auto&&, auto&&) = delete;

    struct discount_factor_t {
        template <egret::cpt::yield_curve C>
        constexpr auto operator()(const C& curve, const std::chrono::sys_days& from, const std::chrono::sys_days& to) const
        {
            using std::exp;
            constexpr double oneday = 1. / 365.;
            const auto dcf = (to - from).count() * oneday;
            auto fwd = egret::model::forward_rate(curve, from, to);
            return exp(- std::move(fwd) * dcf);
        }
    };

} // namespace egret_detail::df_impl

namespace egret::model::inline cpo {
// -----------------------------------------------------------------------------
//  [cpo] discount_factor
// -----------------------------------------------------------------------------
    inline constexpr auto discount_factor = egret_detail::df_impl::discount_factor_t {};

} // namespace egret::model
