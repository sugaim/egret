#pragma once

#include <chrono>
#include <optional>
#include <nlohmann/json_fwd.hpp>
#include "core/utils/string_utils/to_string.h"
#include "core/utils/json_utils/j2obj.h"
#include "egret/chrono/daycounters/concepts.h"
#include "egret/quantities/rate.h"

namespace egret::inst::cfs {
// -----------------------------------------------------------------------------
//  [struct] floating_rate_cf
// -----------------------------------------------------------------------------    
    /**
     * @brief PV = DF(payment_date) * notional * accrual_daycounter(accrual_start, accrual_end) * rate,
     *  where rate is computed with rate_definition from a projection curve specified by curve_tag
     * 
     * @tparam DiscountTag: discount curve specifier
     * @tparam RateTag: projection curve specifier
     * @tparam RateDef: rate definition, which includes term, spread and so on.
     * @tparam DC: daycounter
     * @tparam N: notional
    */
    template <
        typename DiscountTag, typename RateTag, typename RateDef,
        typename DC, typename N = double
    >
    struct floating_rate_cf {
        DiscountTag discount_tag;
        RateTag curve_tag;
        RateDef rate_definition;
        N notional;
        std::chrono::sys_days accrual_start = {};
        std::chrono::sys_days accrual_end = {};
        std::chrono::sys_days fixing_date = {};
        std::chrono::sys_days payment_date = {};
        std::chrono::sys_days cashout_date = {};
        DC accrual_daycounter;
        std::optional<rate<double>> fixed_coupon_rate;
    };

} // namespace egret::inst::cfs

namespace nlohmann {
    template <typename DiscountTag, typename RateTag, typename RateDef, typename DC, typename N>
    struct adl_serializer<egret::inst::cfs::floating_rate_cf<DiscountTag, RateTag, RateDef, DC, N>> {

        using target_type = egret::inst::cfs::floating_rate_cf<DiscountTag, RateTag, RateDef, DC, N>;

        static constexpr auto deser = egret::util::j2obj::construct<target_type>(
            "discount_tag" >> egret::util::j2obj::get<DiscountTag>,
            "curve_tag" >> egret::util::j2obj::get<RateTag>,
            "rate_definition" >> egret::util::j2obj::get<RateDef>,
            "notional" >> egret::util::j2obj::get<N>,
            "accrual_start" >> egret::util::j2obj::string.parse_to<std::chrono::sys_days>("%F"),
            "accrual_end" >> egret::util::j2obj::string.parse_to<std::chrono::sys_days>("%F"),
            "fixing_date" >> egret::util::j2obj::string.parse_to<std::chrono::sys_days>("%F"),
            "payment_date" >> egret::util::j2obj::string.parse_to<std::chrono::sys_days>("%F"),
            "cashout_date" >> egret::util::j2obj::string.parse_to<std::chrono::sys_days>("%F"),
            "accrual_daycounter" >> egret::util::j2obj::get<DC>,
            "fixed_coupon_rate" >> egret::util::j2obj::get<egret::rate<double>>.optional()
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
                std::is_assignable_v<Json&, const RateTag&> &&
                std::is_assignable_v<Json&, const RateDef&> &&
                std::is_assignable_v<Json&, const N&> &&
                std::is_assignable_v<Json&, const DC&> &&
                std::is_assignable_v<Json&, const egret::rate<R>&>
        static void to_json(Json& j, const target_type& cf)
        {
            namespace util = egret::util;
            
            j["discount_tag"] = cf.discount_tag;
            j["curve_tag"] = cf.curve_tag;
            j["rate_definition"] = cf.rate_definition;
            j["notional"] = cf.notional;
            j["accrual_start"] = util::to_string(cf.accrual_start);
            j["accrual_end"] = util::to_string(cf.accrual_end);
            j["fixing_date"] = util::to_string(cf.fixing_date);
            j["payment_date"] = util::to_string(cf.payment_date);
            j["cashout_date"] = util::to_string(cf.cashout_date);
            j["accrual_daycounter"] = cf.accrual_daycounter;
            if (cf.fixed_coupon_rate) {
                j["fixed_coupon_rate"] = *cf.fixed_coupon_rate;
            }
        }
    };

} // namespace nlohmann
