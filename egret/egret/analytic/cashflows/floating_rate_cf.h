#pragma once

#include <chrono>
#include "core/assertions/assertion.h"
#include "egret/models/curves/concepts.h"
#include "egret/models/curves/discount_factor.h"
#include "egret/markets/daycounters/concepts.h"
#include "egret/instruments/cashflows/floating_rate_cf.h"

namespace egret::analytic {
// -----------------------------------------------------------------------------
//  [struct] floating_coupon_rate_calculator
// -----------------------------------------------------------------------------
    template <cpt::yield_curve Curve, typename RateDef>
    struct floating_coupon_rate_calculator {};

// -----------------------------------------------------------------------------
//  [fn] evaluate
// -----------------------------------------------------------------------------
    template <
        typename DiscountTag, typename ProjTag, typename RateDef,
        cpt::daycounter DC, typename N, typename R, 
        typename RateTag, cpt::yield_curve Curve
    >
        requires
            std::strict_weak_order<std::less<>, DiscountTag, RateTag> &&
            std::strict_weak_order<std::less<>, ProjTag, RateTag> &&
            requires (
                const floating_coupon_rate_calculator<Curve, RateDef>& calculator,
                const Curve& curve, const RateDef& rate_def, 
                const std::chrono::sys_days& acc_stt,
                const std::chrono::sys_days& acc_end
            ) {
                { calculator(curve, rate_def, acc_stt, acc_end) } -> cpt::non_void;
            }
    constexpr model::forward_rate_t<Curve> evaluate(
        const inst::cfs::floating_rate_cf<DiscountTag, ProjTag, RateDef, DC, N, R>& cf,
        const std::chrono::sys_days& vdt,
        const std::map<RateTag, Curve>& curves
    )
    {
        egret::assertion(curves.contains(cf.discount_tag), "Discount curve is not found.");
        egret::assertion(curves.contains(cf.curve_tag), "Projection curve is not found.");
        if (cf.cashout_date <= vdt) {
            return static_cast<model::forward_rate_t<Curve>>(0);
        }
        const auto& dcurve = *(curves.find(cf.discount_tag));
        const auto& pcurve = *(curves.find(cf.curve_tag));
        const auto& rate_def = cf.rate_definition;

        const double dcf = mkt::dcf(cf.accrual_daycounter, cf.accrual_start, cf.accrual_end);
        auto df = model::discount_factor(dcurve, vdt, cf.payment_date);

        if (cf.fixed_coupon_rate) {
            return cf.notional * cf.fixed_coupon_rate->value() * dcf * std::move(df);
        }
        else {
            constexpr auto calculator = floating_coupon_rate_calculator<Curve, RateDef> {};
            auto coupon_rate = calculator(pcurve, rate_def, cf.accrual_start, cf.accrual_end);
            return cf.notional * std::move(coupon_rate) * dcf * std::move(df);
        }
    }

} // namespace egret::analytic
