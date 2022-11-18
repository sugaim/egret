#pragma once

#include <chrono>
#include <nlohmann/json_fwd.hpp>
#include "core/utils/string_utils/to_string.h"
#include "core/utils/json_utils/j2obj.h"
#include "egret/quantities/rate.h"
#include "leg.h"

namespace egret::inst::cfs {
// -----------------------------------------------------------------------------
//  [struct] fixed_leg_header
// -----------------------------------------------------------------------------
    template <typename DiscountTag, typename DC, typename N = double>
    struct fixed_leg_header {
        DiscountTag discount_curve;
        DC accrual_daycounter;
        N notional;
    };

// -----------------------------------------------------------------------------
//  [struct] fixed_rate_cf
// -----------------------------------------------------------------------------    
    template <typename N = double, typename R = double>
    struct fixed_rate_cf {
        N notional_ratio;
        std::chrono::sys_days accrual_start = {};
        std::chrono::sys_days accrual_end = {};
        std::chrono::sys_days payment_date = {};
        std::chrono::sys_days cashout_date = {};
        std::chrono::sys_days entitlement_date = {};
        rate<R> rate = {};
    };

// -----------------------------------------------------------------------------
//  [type] fixed_leg
// -----------------------------------------------------------------------------
    template <typename DiscountTag, typename DC, typename N = double, typename NR = N, typename R = double>
    using fixed_leg = leg<
        fixed_leg_header<DiscountTag, DC, N>,
        fixed_rate_cf<NR, R>
    >;

} // namespace egret::inst::cfs

namespace nlohmann {
    template <typename DiscountTag, typename DC, typename N>
    struct adl_serializer<egret::inst::cfs::fixed_leg_header<DiscountTag, DC, N>> {
        
        using target_type = egret::inst::cfs::fixed_leg_header<DiscountTag, DC, N>;

        static constexpr auto deser = egret::util::j2obj::construct<target_type>(
            "discount_curve" >> egret::util::j2obj::get<DiscountTag>,
            "accrual_daycounter" >> egret::util::j2obj::get<DC>,
            "notional" >> egret::util::j2obj::get<N>
        );

        template <typename Json>
            requires requires (const Json& j) { deser(j); }
        static target_type from_json(const Json& j)
        {
            return deser(j);
        }

        template <typename Json>
            requires
                std::is_assignable_v<Json&, const DiscountTag&> &&
                std::is_assignable_v<Json&, const N&> &&
                std::is_assignable_v<Json&, const DC&>
        static void to_json(Json& j, const target_type& obj)
        {
            namespace util = egret::util;
            
            j["discount_curve"] = obj.discount_curve;
            j["accrual_daycounter"] = obj.accrual_daycounter;
            j["notional"] = obj.notional;
        }
    };

    template <typename N, typename R>
    struct adl_serializer<egret::inst::cfs::fixed_rate_cf<N, R>> {
        
        using target_type = egret::inst::cfs::fixed_rate_cf<N, R>;

        static constexpr auto deser = egret::util::j2obj::construct<target_type>(
            "notional_ratio" >> egret::util::j2obj::get<N>.optional().value_or(1),
            "accrual_start" >> egret::util::j2obj::string.parse_to<std::chrono::sys_days>("%F"),
            "accrual_end" >> egret::util::j2obj::string.parse_to<std::chrono::sys_days>("%F"),
            "payment_date" >> egret::util::j2obj::string.parse_to<std::chrono::sys_days>("%F"),
            "cashout_date" >> egret::util::j2obj::string.parse_to<std::chrono::sys_days>("%F"),
            "entitlement_date" >> egret::util::j2obj::string.parse_to<std::chrono::sys_days>("%F"),
            "rate" >> egret::util::j2obj::get<egret::rate<R>>
        );

        template <typename Json>
            requires requires (const Json& j) { deser(j); }
        static target_type from_json(const Json& j)
        {
            return deser(j);
        }

        template <typename Json>
            requires
                std::is_assignable_v<Json&, const N&> &&
                std::is_assignable_v<Json&, const egret::rate<R>&>
        static void to_json(Json& j, const target_type& obj)
        {
            namespace util = egret::util;
            
            j["notional_ratio"] = obj.notional_ratio;
            j["accrual_start"] = util::to_string(obj.accrual_start);
            j["accrual_end"] = util::to_string(obj.accrual_end);
            j["payment_date"] = util::to_string(obj.payment_date);
            j["cashout_date"] = util::to_string(obj.cashout_date);
            j["entitlement_date"] = util::to_string(obj.entitlement_date);
            j["rate"] = obj.rate;
        }
    };

} // namespace nlohmann
