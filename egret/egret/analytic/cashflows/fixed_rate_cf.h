#pragma once

#include <map>
#include <chrono>
#include "core/assertions/assertion.h"
#include "egret/models/curves/concepts.h"
#include "egret/models/curves/discount_factor.h"
#include "egret/chrono/daycounters/concepts.h"
#include "egret/instruments/cashflows/fixed_rate_cf.h"

namespace egret::analytic {
// -----------------------------------------------------------------------------
//  [fn] evaluate
// -----------------------------------------------------------------------------
    template <
        typename DiscountTag, cpt::daycounter DC, typename N, typename R,
        typename RateTag, cpt::yield_curve Curve
    >
        requires
            std::strict_weak_order<std::less<>, DiscountTag, RateTag>
    constexpr model::forward_rate_t<Curve> evaluate(
        const inst::cfs::fixed_rate_cf<DiscountTag, DC, N, R>& cf,
        const std::chrono::sys_days& vdt,
        const std::map<RateTag, Curve>& curves
    )
    {
        egret::assertion(curves.contains(cf.discount_tag), "Discount curve is not found.");
        if (cf.cashout_date <= vdt) {
            return static_cast<model::forward_rate_t<Curve>>(0);
        }
        const auto& curve = *(curves.find(cf.discount_tag));
        const double dcf = chrono::dcf(cf.accrual_daycounter, cf.accrual_start, cf.accrual_end);
        auto df = model::discount_factor(curve, vdt, cf.payment_date);
        return cf.notional * cf.rate.value() * dcf * std::move(df);
    }

} // namespace egret::analytic
