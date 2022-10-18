#pragma once

#include <chrono>
#include <nlohmann/json_fwd.hpp>
#include "egret/markets/rate.h"
#include "core/utils/string_utils/to_string.h"
#include "core/utils/json_utils/j2obj.h"
#include "egret/markets/daycounters/concepts.h"

namespace egret::inst {
// -----------------------------------------------------------------------------
//  [struct] float_rate_cf
// -----------------------------------------------------------------------------    
    /**
     * @brief PV = DF(payment_date) * notional * accrual_daycounter(accrual_start, accrual_end) * (gearing * rate + spread),
     * where rate is floating rate specified by rate_tag
     * 
     * @tparam RateTag: rate specifier
     * @tparam DC: daycounter
     * @tparam N: notional 
     * @tparam G: gearing 
     * @tparam R: spread
    */
    template <typename RateTag, typename DC, typename N = double, typename G = double, typename R = G>
    struct float_rate_cf {
        RateTag rate_tag;
        N notional;
        std::chrono::sys_days accrual_start = {};
        std::chrono::sys_days accrual_end = {};
        std::chrono::sys_days fixing_date = {};
        std::chrono::sys_days payment_date = {};
        DC accrual_daycounter;
        G gearing;
        mkt::rate<R> spread = {};
    };

} // namespace egret::inst

namespace nlohmann {
    template <typename RateTag, typename DC, typename N, typename G, typename R>
    struct adl_serializer<egret::inst::float_rate_cf<RateTag, DC, N, G, R>> {
        template <typename Json>
        static egret::inst::float_rate_cf<RateTag, DC, N, G, R> from_json(const Json& j)
        {
            namespace util = egret::util;
            namespace j2obj = util::j2obj;
            constexpr auto deser = j2obj::construct<egret::inst::float_rate_cf<RateTag, DC, N, G, R>>(
                "rate_tag" >> j2obj::get<RateTag>,
                "notional" >> j2obj::get<N>,
                "accrual_start" >> j2obj::string.parse_to<std::chrono::sys_days>("%F"),
                "accrual_end" >> j2obj::string.parse_to<std::chrono::sys_days>("%F"),
                "fixing_date" >> j2obj::string.parse_to<std::chrono::sys_days>("%F"),
                "payment_date" >> j2obj::string.parse_to<std::chrono::sys_days>("%F"),
                "accrual_daycounter" >> j2obj::get<DC>,
                "gearing" >> j2obj::get<G>,
                "spread" >> j2obj::get<egret::mkt::rate<R>>
            );
            return deser(j);
        }

        template <typename Json>
        static void to_json(Json& j, const egret::inst::float_rate_cf<RateTag, DC, N, G, R>& cf)
        {
            namespace util = egret::util;

            j["rate_tag"] = cf.rate_tag;
            j["notional"] = cf.notional;
            j["accrual_start"] = util::to_string(cf.accrual_start);
            j["accrual_end"] = util::to_string(cf.accrual_end);
            j["fixing_date"] = util::to_string(cf.fixing_date);
            j["payment_date"] = util::to_string(cf.payment_date);
            j["accrual_daycounter"] = cf.accrual_daycounter;
            j["gearing"] = cf.gearing;
            j["spread"] = cf.spread;
        }
    };

} // namespace nlohmann
