#pragma once

#include <nlohmann/json_fwd.hpp>
#include "core/utils/json_utils/j2obj.h"
#include "egret/quantities/rate.h"
#include "leg.h"

namespace egret::inst::cfs {
// -----------------------------------------------------------------------------
//  [struct] term_rate_leg_header
// -----------------------------------------------------------------------------
    template <typename DiscountTag, typename RateTag, typename DC, typename N = double>
    struct term_rate_leg_header {
        DiscountTag discount_curve;
        RateTag projection_curve;
        DC rate_daycounter;
        DC accrual_daycounter;
        N notional;
    };

// -----------------------------------------------------------------------------
//  [struct] term_rate_cashflow
// -----------------------------------------------------------------------------
    template <typename N = double, typename G = double, typename R = G>
    struct term_rate_cashflow {
        N notional_ratio;
        std::chrono::sys_days accrual_start = {};
        std::chrono::sys_days accrual_end = {};
        std::chrono::sys_days reference_start;
        std::chrono::sys_days reference_end;
        std::chrono::sys_days fixing_date = {};
        std::chrono::sys_days payment_date = {};
        std::chrono::sys_days cashout_date = {};
        std::chrono::sys_days entitlement_date = {};
        G gearing;
        rate<R> spread;
        std::optional<rate<double>> fixed_coupon_rate;
    };

// -----------------------------------------------------------------------------
//  [type] term_rate_leg
// -----------------------------------------------------------------------------
    template <
        typename DiscountTag, typename RateTag, 
        typename DC, typename N = double, typename NR = N, typename G = double, typename R = G
    >
    using term_rate_leg = leg<
        term_rate_leg_header<DiscountTag, RateTag, DC, N>,
        term_rate_cashflow<NR, G, R>
    >;

} // namespace egret::inst::cfs

namespace nlohmann {
    template <typename DiscountTag, typename RateTag, typename DC, typename N>
    struct adl_serializer<egret::inst::cfs::term_rate_leg_header<DiscountTag, RateTag, DC, N>> {
        
        using target_type = egret::inst::cfs::term_rate_leg_header<DiscountTag, RateTag, DC, N>;

        static constexpr auto deser = egret::util::j2obj::construct<target_type>(
            "discount_curve" >> egret::util::j2obj::get<DiscountTag>,
            "projection_curve" >> egret::util::j2obj::get<RateTag>,
            "rate_daycounter" >> egret::util::j2obj::get<DC>,
            "accrual_daycounter" >> egret::util::j2obj::get<DC>,
            "notional" >> egret::util::j2obj::get<N>
        );
        template <typename Json>
            requires requires (const Json& j) { deser(j); }
        static target_type from_json(const Json& json)
        {
            return deser(json);
        }

        template <typename Json>
            requires
                std::is_assignable_v<Json&, const DiscountTag&> &&
                std::is_assignable_v<Json&, const RateTag&> &&
                std::is_assignable_v<Json&, const DC&> &&
                std::is_assignable_v<Json&, const N&>
        static void to_json(Json& j, const target_type& obj)
        {
            namespace util = egret::util;
            
            j["discount_curve"] = obj.discount_curve;
            j["projection_curve"] = obj.projection_curve;
            j["rate_daycounter"] = obj.rate_daycounter;
            j["accrual_daycounter"] = obj.accrual_daycounter;
            j["notional"] = obj.notional;
        }
    };

    template <typename N, typename G, typename R>
    struct adl_serializer<egret::inst::cfs::term_rate_cashflow<N, G, R>> {

        using target_type = egret::inst::cfs::term_rate_cashflow<N, G, R>;

        static constexpr auto deser = egret::util::j2obj::construct<target_type>(
            "notional_ratio" >> egret::util::j2obj::get<N>.optional().value_or(1),
            "accrual_start" >> egret::util::j2obj::string.parse_to<std::chrono::sys_days>("%F"),
            "accrual_end" >> egret::util::j2obj::string.parse_to<std::chrono::sys_days>("%F"),
            "reference_start" >> egret::util::j2obj::string.parse_to<std::chrono::sys_days>("%F"),
            "reference_end" >> egret::util::j2obj::string.parse_to<std::chrono::sys_days>("%F"),
            "fixing_date" >> egret::util::j2obj::string.parse_to<std::chrono::sys_days>("%F"),
            "payment_date" >> egret::util::j2obj::string.parse_to<std::chrono::sys_days>("%F"),
            "cashout_date" >> egret::util::j2obj::string.parse_to<std::chrono::sys_days>("%F"),
            "entitlement_date" >> egret::util::j2obj::string.parse_to<std::chrono::sys_days>("%F"),
            "gearing" >> egret::util::j2obj::get<G>.optional().value_or(1),
            "spread" >> egret::util::j2obj::get<egret::rate<R>>.optional().value_or(0),
            "fixed_coupon_rate" >> egret::util::j2obj::get<egret::rate<double>>.optional()
        );

        template <typename Json>
            requires requires (const Json& j) { deser(j); }
        static target_type from_json(const Json& j)
        {
            target_type result = deser(j);
            if (result.accrual_end < result.accrual_start) {
                throw egret::exception(
                    "Invalid term_rate_cashflow because its accrual start/end dates "
                    "are in invalid order. [accrual_start='{}', accrual_end='{}']",
                    result.accrual_start, result.accrual_end
                );
            }
            if (result.reference_end < result.reference_start) {
                throw egret::exception(
                    "Invalid term_rate_cashflow because its reference start/end dates "
                    "are in invalid order. [reference_start='{}', reference_end='{}']",
                    result.reference_start, result.reference_end
                );
            }
            return result;
        }

        template <typename Json>
            requires
                std::is_assignable_v<Json&, const N&> &&
                std::is_assignable_v<Json&, const G&> &&
                std::is_assignable_v<Json&, const R&>
        static void to_json(Json& j, const target_type& obj)
        {
            namespace util = egret::util;
            
            j["notional_ratio"] = obj.notional_ratio;
            j["accrual_start"] = util::to_string(obj.accrual_start);
            j["accrual_end"] = util::to_string(obj.accrual_end);
            j["reference_start"] = util::to_string(obj.reference_start);
            j["reference_end"] = util::to_string(obj.reference_end);
            j["fixing_date"] = util::to_string(obj.fixing_date);
            j["payment_date"] = util::to_string(obj.payment_date);
            j["cashout_date"] = util::to_string(obj.cashout_date);
            j["entitlement_date"] = util::to_string(obj.entitlement_date);
            j["gearing"] = obj.gearing;
            j["spread"] = obj.spread;
            if (obj.fixed_coupon_rate) {
                j["fixed_coupon_rate"] = *(obj.fixed_coupon_rate);
            }
        }
    };

} // namespace nlohmann