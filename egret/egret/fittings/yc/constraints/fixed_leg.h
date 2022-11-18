#pragma once

#include "core/utils/string_utils/to_string.h"
#include "egret/chrono/daycounters/concepts.h"
#include "egret/models/curves/discount_factor.h"
#include "egret/instruments/cashflows/fixed_leg.h"
#include "swap_constraint.h"

namespace egret::fit::yc {
// -----------------------------------------------------------------------------
//  [struct] cashflow_evaluator
// -----------------------------------------------------------------------------
    template <typename DiscountTag, cpt::daycounter DC>
    struct cashflow_evaluator<inst::cfs::fixed_leg_header<DiscountTag, DC>, inst::cfs::fixed_rate_cf<>> {
        
        using header_t = inst::cfs::fixed_leg_header<DiscountTag, DC>;
        using cashflow_t = inst::cfs::fixed_rate_cf<>;

        template <typename RateTag, cpt::yield_curve Curve>
            requires std::strict_weak_order<std::less<>, const RateTag&, const DiscountTag&>
        model::forward_rate_t<Curve> operator()(
            const header_t& header, const cashflow_t& cashflow,
            const std::chrono::sys_days& vdt, 
            const std::map<RateTag, Curve>& curves
        ) const
        {
            if (!curves.contains(header.discount_curve)) {
                if constexpr (cpt::to_stringable<DiscountTag>) {
                    throw exception("Discount curve '{}' is not found.", util::to_string(header.discount_curve))
                        .record_stacktrace();
                }
                else {
                    throw exception("Discount curve is not found.")
                        .record_stacktrace();
                }
            }
            using result_t = model::forward_rate_t<Curve>;
            if (cashflow.cashout_date <= vdt) {
                return static_cast<result_t>(0);
            }
            const double notional = header.notional * cashflow.notional_ratio;
            const double dcf = chrono::dcf(header.accrual_daycounter, std::min(cashflow.accrual_start, vdt), cashflow.accrual_end);
            auto df = model::discount_factor(curves.at(header.discount_curve), vdt, cashflow.payment_date);

            return static_cast<result_t>(std::move(df) * (notional * dcf * cashflow.rate));
        }
    };

} // namespace egret::fit::yc
