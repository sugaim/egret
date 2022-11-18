#pragma once

#include "core/utils/string_utils/to_string.h"
#include "egret/chrono/daycounters/concepts.h"
#include "egret/models/curves/discount_factor.h"
#include "egret/instruments/cashflows/term_rate_leg.h"
#include "swap_constraint.h"

namespace egret::fit::yc {
// -----------------------------------------------------------------------------
//  [struct] cashflow_evaluator
// -----------------------------------------------------------------------------
    template <typename DiscountTag, typename FloatingTag, typename DC>
    struct cashflow_evaluator<inst::cfs::term_rate_leg_header<DiscountTag, FloatingTag, DC>, inst::cfs::term_rate_cashflow<>> {
        
        using header_t = inst::cfs::term_rate_leg_header<DiscountTag, FloatingTag, DC>;
        using cashflow_t = inst::cfs::term_rate_cashflow<>;

        template <typename RateTag, cpt::yield_curve Curve>
            requires 
                std::strict_weak_order<std::less<>, const RateTag&, const DiscountTag&> &&
                std::strict_weak_order<std::less<>, const RateTag&, const FloatingTag&>
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
            if (!curves.contains(header.projection_curve)) {
                if constexpr (cpt::to_stringable<FloatingTag>) {
                    throw exception("Projection curve '{}' is not found.", util::to_string(header.projection_curve))
                        .record_stacktrace();
                }
                else {
                    throw exception("Projection curve is not found.")
                        .record_stacktrace();
                }
            }

            using result_t = model::forward_rate_t<Curve>;
            if (cashflow.cashout_date <= vdt || cashflow.reference_start == cashflow.reference_end) {
                return static_cast<result_t>(0);
            }
            const double notional = header.notional * cashflow.notional_ratio;
            const double dcf = chrono::dcf(header.accrual_daycounter, std::min(cashflow.accrual_start, vdt), cashflow.accrual_end);
            auto df = model::discount_factor(curves.at(header.discount_curve), vdt, cashflow.payment_date);

            if (cashflow.fixed_coupon_rate) {
                const double cpn = *(cashflow.fixed_coupon_rate);
                return static_cast<result_t>(std::move(df) * (notional * dcf * cpn));
            }


            const auto& pcurve = curves.at(header.projection_curve);
            auto pdf = model::discount_factor(pcurve, cashflow.reference_start, cashflow.reference_start);
            auto pdcf = chrono::dcf(header.rate_daycounter, cashflow.reference_start, cashflow.reference_end);
            auto rate = (1 / std::move(pdf) - 1) / pdcf;

            auto cpn = std::move(rate) * cashflow.gearing + cashflow.spread.value();

            return static_cast<result_t>(std::move(df) * (notional * dcf * std::move(cpn)));
        }
    };

} // namespace egret::fit::yc
