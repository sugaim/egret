#pragma once

#include "core/assertions/assertion.h"
#include "egret/models/curves/concepts.h"
#include "egret/models/curves/discount_factor.h"
#include "egret/instruments/cashflows/term_rate_cf.h"

namespace egret::fit::yc {
// -----------------------------------------------------------------------------
//  [fn] evaluate
// -----------------------------------------------------------------------------
    template <
        typename DiscountTag, typename ProjTag, cpt::daycounter DC, typename N, typename G, typename R, 
        typename RateTag, cpt::yield_curve Curve
    >
    constexpr model::forward_rate_t<Curve> evaluate(
        const inst::cfs::term_rate_cf<DiscountTag, ProjTag, DC, N, G, R>& cf,
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

        if (cf.fixing_rate) {
            auto coupon_rate = std::move(*cf.fixing_rate) * rate_def.gearing + rate_def.spread.value();
            return cf.notional * std::move(coupon_rate) * dcf * std::move(df);
        }

        auto rate = [&pcurve, &rate_def] () -> model::forward_rate_t<Curve> {
            const auto& stt = rate_def.reference_start;
            const auto& end = rate_def.reference_end;
            if (stt == end) {
                return model::forward_rate(pcurve, stt, end);
            }
            const double dcf = mkt::dcf(rate_def.rate_daycounter, stt, end);
            auto fwd_df = model::discount_factor(pcurve, stt, end);
            return (1. / std::move(fwd_df) - 1.) / dcf;
        }();
        auto coupon_rate = std::move(rate) * rate_def.gearing + rate_def.spread.value();
        return cf.notional * std::move(coupon_rate) * dcf * std::move(df);
    }

} // namespace egret::fit::yc
