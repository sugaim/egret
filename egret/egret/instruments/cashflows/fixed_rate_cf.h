#pragma once

#include <chrono>
#include <nlohmann/json_fwd.hpp>
#include "core/utils/string_utils/to_string.h"
#include "core/utils/json_utils/j2obj.h"
#include "egret/markets/rate.h"

namespace egret::inst {
// -----------------------------------------------------------------------------
//  [struct] fixed_rate_cf
// -----------------------------------------------------------------------------    
    /**
     * @brief PV = DF(payment_date) * notional * accrual_daycounter(accrual_start, accrual_end) * rate
     * 
     * @tparam DC: daycounter
     * @tparam N: notional
     * @tparam R: rate
    */
    template <typename DC, typename N = double, typename R = double>
    struct fixed_rate_cf {
        N notional;
        std::chrono::sys_days accrual_start = {};
        std::chrono::sys_days accrual_end = {};
        std::chrono::sys_days payment_date = {};
        DC accrual_daycounter;
        mkt::rate<R> rate = {};
    };

} // namespace egret::inst

namespace nlohmann {
    template <typename DC, typename N, typename R>
    struct adl_serializer<egret::inst::fixed_rate_cf<DC, N, R>> {
        template <typename Json>
        static egret::inst::fixed_rate_cf<DC, N, R> from_json(const Json& j)
        {
            namespace util = egret::util;
            namespace j2obj = util::j2obj;
            constexpr auto deser = j2obj::object.from_properties<egret::inst::fixed_rate_cf<DC, N, R>>(
                "notional" >> j2obj::get<N>,
                "accrual_start" >> j2obj::string.parse_to<std::chrono::sys_days>("%F"),
                "accrual_end" >> j2obj::string.parse_to<std::chrono::sys_days>("%F"),
                "payment_date" >> j2obj::string.parse_to<std::chrono::sys_days>("%F"),
                "accrual_daycounter" >> j2obj::get<DC>,
                "rate" >> j2obj::get<egret::mkt::rate<R>>
            );
            return deser(j);
        }

        template <typename Json>
        static void to_json(Json& j, const egret::inst::fixed_rate_cf<DC, N, R>& cf)
        {
            namespace util = egret::util;
        
            j["notional"] = cf.notional;
            j["accrual_start"] = util::to_string(cf.accrual_start);
            j["accrual_end"] = util::to_string(cf.accrual_end);
            j["payment_date"] = util::to_string(cf.payment_date);
            j["accrual_daycounter"] = cf.accrual_daycounter;
            j["rate"] = cf.rate;
        }
    };

} // namespace nlohmann