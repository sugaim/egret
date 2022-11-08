#pragma once

#include <nlohmann/json_fwd.hpp>
#include "core/utils/json_utils/j2obj.h"
#include "egret/markets/rate.h"
#include "float_rate_cf.h"

namespace egret::inst::cfs {
// -----------------------------------------------------------------------------
//  [struct] term_rate_definition
// -----------------------------------------------------------------------------
    template <typename DC, typename G = double, typename R = G>
    struct term_rate_definition {
        DC rate_daycounter;
        std::chrono::sys_days reference_start;
        std::chrono::sys_days reference_end;
        G gearing;
        mkt::rate<R> spread;
    };

// -----------------------------------------------------------------------------
//  [type] term_rate_cf
// -----------------------------------------------------------------------------
    template <
        typename DiscountTag, typename RateTag, 
        typename DC, typename N = double, typename G = double, typename R = G
    >
    using term_rate_cf = float_rate_cf<
        DiscountTag, RateTag, term_rate_definition<DC, G, R>, DC, N>;

} // namespace egret::inst::cfs

namespace nlohmann {
    template <typename DC, typename G, typename R>
    struct adl_serializer<egret::inst::cfs::term_rate_definition<DC, G, R>> {
        
        using target_type = egret::inst::cfs::term_rate_definition<DC, G, R>;

        template <typename Json>
        static target_type from_json(const Json& json)
        {
            namespace util = egret::util;
            namespace j2obj = util::j2obj;
            constexpr auto deser = j2obj::construct<target_type>(
                "rate_daycounter" >> j2obj::get<DC>,
                "reference_start" >> j2obj::string.parse_to<std::chrono::sys_days>("%F"),
                "reference_end" >> j2obj::string.parse_to<std::chrono::sys_days>("%F"),
                "gearing" >> j2obj::get<G>,
                "spread" >> j2obj::get<R>
            );
            return deser(j);
        }

        template <typename Json>
        static void to_json(Json& j, const target_type& cf)
        {
            namespace util = egret::util;
            
            j["rate_daycounter"] = cf.rate_daycounter;
            j["reference_start"] = cf.reference_start;
            j["reference_end"] = cf.reference_start;
            j["gearing"] = cf.gearing;
            j["spread"] = cf.spread;
        }
    };

} // namespace nlohmann