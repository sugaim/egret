#pragma once

#include "core/utils/string_utils/to_string.h"
#include "egret/chrono/daycounters/concepts.h"
#include "egret/chrono/adjustments/add_bd.h"
#include "egret/chrono/adjustments/prev_bd.h"
#include "egret/chrono/adjustments/next_bd.h"
#include "egret/models/curves/discount_factor.h"
#include "egret/instruments/cashflows/overnight_index_leg.h"
#include "swap_constraint.h"

namespace egret::fit::yc {
// -----------------------------------------------------------------------------
//  [struct] cashflow_evaluator
// -----------------------------------------------------------------------------
    template <typename DiscountTag, typename RateTag, typename DC>
    struct cashflow_evaluator<inst::cfs::overnight_index_leg_header<DiscountTag, RateTag, DC>, inst::cfs::overnight_index_cashflow<>> {
        
        using header_t = inst::cfs::overnight_index_leg_header<DiscountTag, RateTag, DC>;
        using cashflow_t = inst::cfs::overnight_index_cashflow<>;

        template <typename RateTag, cpt::yield_curve Curve>
            requires 
                std::strict_weak_order<std::less<>, const RateTag&, const DiscountTag&> &&
                std::strict_weak_order<std::less<>, const RateTag&, const RateTag&>
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
                if constexpr (cpt::to_stringable<RateTag>) {
                    throw exception("Projection curve '{}' is not found.", util::to_string(header.projection_curve))
                        .record_stacktrace();
                }
                else {
                    throw exception("Projection curve is not found.")
                        .record_stacktrace();
                }
            }

            using result_t = model::forward_rate_t<Curve>;
            if (cashflow.cashout_date <= vdt || cashflow.accrual_start == cashflow.accrual_end) {
                return static_cast<result_t>(0);
            }
            const double notional = header.notional * cashflow.notional_ratio;
            const double dcf = chrono::dcf(header.accrual_daycounter, std::min(cashflow.accrual_start, vdt), cashflow.accrual_end);
            auto df = model::discount_factor(curves.at(header.discount_curve), vdt, cashflow.payment_date);

            if (cashflow.fixed_coupon_rate) {
                const double cpn = *(cashflow.fixed_coupon_rate);
                return static_cast<result_t>(std::move(df) * (notional * dcf * cpn);
            }

            const auto& pcurve = curves.at(header.projection_curve);
            const auto next = chrono::next_bd(header.rate_reference_calendar);

            const auto per_end = cashflow.accrual_end | chrono::add_bd(-cashflow.backward_shift - cashflow.lookback, header.rate_reference_calendar);
            const auto ref_end = per_end | chrono::add_bd(-cashflow.lockout, header.rate_reference_calendar);
            const auto weight_start = cashflow.accrual_start | chrono::add_bd(-cashflow.backward_shift, header.rate_reference_calendar);
            auto weight_cursor = weight_start;
            auto rate_cursor = weight_cursor | chrono::add_bd(-cashflow.lookback, header.rate_reference_calendar);

            result_t rate = 0;
            while (rate_cursor < ref_end) {
                const auto rate_stt = rate_cursor;
                const auto weight_stt = weight_cursor;
                rate_cursor = next(rate_cursor);
                weight_cursor = next(weight_cursor);
                auto pdf = model::discount_factor(pcurve, rate_stt, rate_cursor);
                const auto fwd_dcf = chrono::dcf(header.rate_daycounter, rate_stt, rate_cursor);
                const auto pdcf = chrono::dcf(header.rate_daycounter, weight_stt, weight_cursor);
                rate *= 1 + (1 / std::move(pdf) - 1) / fwd_dcf * pdcf;
            }
            const auto locked_rate = [&pcurve, &rate_cursor, &next, &header]{
                const auto next_date = rate_cursor | next;
                auto pdf = model::discount_factor(pcurve, rate_cursor, next_date);
                const auto fwd_dcf = chrono::dcf(header.rate_daycounter, rate_cursor, next_date);
                const auto pdcf = chrono::dcf(header.rate_daycounter, rate_cursor, next_date);
                return (1 / std::move(pdf) - 1) / fwd_dcf;
            }();
            while (rate_cursor < per_end) {
                const auto weight_stt = weight_cursor;
                rate_cursor = next(rate_cursor);
                weight_cursor = next(weight_cursor);
                const auto pdcf = chrono::dcf(header.rate_daycounter, weight_stt, weight_cursor);
                rate *= 1 + locked_rate * pdcf;
            }

            rate = (std::move(rate) - 1) / chrono::dcf(header.rate_daycounter, weight_start, weight_cursor);
            auto cpn = std::move(rate) * cashflow.gearing + cashflow.spread.value();

            return static_cast<result_t>(std::move(df) * (notional * dcf * std::move(cpn));
        }
    };

} // namespace egret::fit::yc
