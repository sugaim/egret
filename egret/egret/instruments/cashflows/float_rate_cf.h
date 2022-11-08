#pragma once

#include <chrono>
#include <optional>
#include <nlohmann/json_fwd.hpp>
#include "core/utils/string_utils/to_string.h"
#include "core/utils/json_utils/j2obj.h"
#include "egret/markets/rate.h"
#include "egret/markets/daycounters/concepts.h"

namespace egret::inst::cfs {
// -----------------------------------------------------------------------------
//  [struct] float_rate_cf
// -----------------------------------------------------------------------------    
    /**
     * @brief PV = DF(payment_date) * notional * accrual_daycounter(accrual_start, accrual_end) * rate,
     *  where rate is computed with rate_definition from a projection curve specified by curve_tag
     * 
     * @tparam DiscountTag: discount curve specifier
     * @tparam RateTag: projection curve specifier
     * @tparam RateDef: rate definition, which includes term, spread and so on.
     * @tparam N: notional
     * @tparam DC: daycounter
    */
    template <
        typename DiscountTag, typename RateTag, typename RateDef,
        typename DC, typename N = double, typename R = double
    >
    struct float_rate_cf {
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
        std::optional<mkt::rate<R>> fixing_rate;
    };

} // namespace egret::inst::cfs

namespace nlohmann {
    template <typename DiscountTag, typename RateTag, typename RateDef, typename DC, typename N, typename R>
    struct adl_serializer<egret::inst::cfs::float_rate_cf<DiscountTag, RateTag, RateDef, DC, N, R>> {

        using target_type = egret::inst::cfs::float_rate_cf<DiscountTag, RateTag, RateDef, DC, N, R>;

        template <typename Json>
        static target_type from_json(const Json& j)
        {
            namespace util = egret::util;
            namespace j2obj = util::j2obj;
            constexpr auto deser = j2obj::construct<target_type>(
                "discount_tag" >> j2obj::get<DiscountTag>,
                "curve_tag" >> j2obj::get<RateTag>,
                "rate_definition" >> j2obj::get<RateDef>,
                "notional" >> j2obj::get<N>,
                "accrual_start" >> j2obj::string.parse_to<std::chrono::sys_days>("%F"),
                "accrual_end" >> j2obj::string.parse_to<std::chrono::sys_days>("%F"),
                "fixing_date" >> j2obj::string.parse_to<std::chrono::sys_days>("%F"),
                "payment_date" >> j2obj::string.parse_to<std::chrono::sys_days>("%F"),
                "cashout_date" >> j2obj::string.parse_to<std::chrono::sys_days>("%F"),
                "accrual_daycounter" >> j2obj::get<DC>,
                "fixing_rate" >> j2obj::get<egret::mkt::rate<R>>.optional()
            );
            return deser(j);
        }

        template <typename Json>
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
            if (cf.fixing_rate) {
                j["fixing_rate"] = *cf.fixing_rate;
            }
        }
    };

} // namespace nlohmann
