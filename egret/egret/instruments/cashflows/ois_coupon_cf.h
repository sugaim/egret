#pragma once

#include <array>
#include <chrono>
#include <string>
#include <optional>
#include <utility>
#include "core/chrono/calendars/calendar.h"
#include "core/chrono/calendars/calendar_server.h"
#include "core/utils/string_utils/string_type_map.h"
#include "core/utils/variant_utils/type_info.h"
#include "core/utils/construct.h"
#include "core/utils/json_utils/j2obj.h"
#include "core/utils/json_utils/variant_to_json.h"
#include "egret/markets/rate.h"
#include "egret/markets/daycounters/concepts.h"

namespace egret::inst {
// -----------------------------------------------------------------------------
//  [struct] ois_observation_convention
//  [fn] from_dto
//  [fn] to_dto
// -----------------------------------------------------------------------------
    template <typename Cal = chrono::calendar>
    struct ois_observation_convention {
        std::chrono::days lockout = {};
        std::chrono::days lookback = {};
        std::chrono::days observation_period_shift = {};

        Cal rate_reference_calendar = {};
        Cal compound_weight_calendar = {};
    };

    ois_observation_convention<chrono::calendar> from_dto(
        const ois_observation_convention<chrono::calendar_identifier>& cnv,
        const chrono::calendar_server& server
    );
    ois_observation_convention<chrono::calendar_identifier> to_dto(
        const ois_observation_convention<chrono::calendar>& cnv
    );

// -----------------------------------------------------------------------------
//  [struct] ois_spread_exclusive_compounding
// -----------------------------------------------------------------------------
    template <typename G = double, typename S = G>
    struct ois_spread_exclusive_compounding {
        G gearing;
        mkt::rate<S> spread;
    };

// -----------------------------------------------------------------------------
//  [struct] ois_flat_compounding
// -----------------------------------------------------------------------------
    template <typename S = double>
    struct ois_flat_compounding {
        mkt::rate<S> spread;
    };

// -----------------------------------------------------------------------------
//  [struct] ois_straight_compounding
// -----------------------------------------------------------------------------
    template <typename S = double>
    struct ois_straight_compounding {
        mkt::rate<S> spread;
    };

// -----------------------------------------------------------------------------
//  [type] ois_compound_convention
// -----------------------------------------------------------------------------
    template <typename G = double, typename S = G>
    using ois_compound_convention = std::variant<
        ois_spread_exclusive_compounding<G, S>,
        ois_flat_compounding<S>,
        ois_straight_compounding<S>
    >;

// -----------------------------------------------------------------------------
//  [struct] ois_coupon_cf
// -----------------------------------------------------------------------------
    /**
     * @brief PV = DF(payment_date) * notional * accrual_daycounter(accrual_start, accrual_end) * rate,
     * where rate is specified by rate_tag, observation_convention and compound_convention.
     * 
     * @tparam RateTag 
     * @tparam DC 
     * @tparam Obs 
     * @tparam Cmp 
     * @tparam N 
    */
    template <
        typename RateTag, typename DC, 
        typename Obs = ois_observation_convention<>, 
        typename Cmp = ois_compound_convention<>,
        typename N = double
    >
    struct ois_coupon_cf {
        RateTag discount_tag;
        RateTag rate_tag;
        N notional;
        std::chrono::sys_days accrual_start = {};
        std::chrono::sys_days accrual_end = {};
        std::chrono::sys_days payment_date = {};        
        DC accrual_daycounter;
        Obs observation_convention;
        Cmp compound_convention;
    };
    
} // namespace egret::inst

namespace nlohmann {
    template <typename Cal>
    struct adl_serializer<egret::inst::ois_observation_convention<Cal>> {
        using target_type = egret::inst::ois_observation_convention<Cal>;

        template <typename Json>
        static target_type from_json(const Json& j)
        {
            namespace util = egret::util;
            namespace j2obj = util::j2obj;
            constexpr auto deser = j2obj::construct<target_type>(
                "lockout" >> j2obj::integer.to<std::chrono::days>(),
                "lookback" >> j2obj::integer.to<std::chrono::days>(),
                "observation_period_shift" >> j2obj::integer.to<std::chrono::days>(),
                "rate_reference_calendar" >> j2obj::get<Cal>,
                "compound_weight_calendar" >> j2obj::get<Cal>
            );
            return deser(j);
        }

        template <typename Json>
        static void to_json(Json& j, const target_type& conv)
        {
            j["lockout"] = conv.lockout.count();
            j["lookback"] = conv.lookback.count();
            j["observation_period_shift"] = conv.observation_period_shift();
            j["rate_reference_calendar"] = j.rate_reference_calendar;
            j["compound_weight_calendar"] = j.compound_weight_calendar;
        }
    };

    template <typename G, typename S>
    struct adl_serializer<egret::inst::ois_spread_exclusive_compounding<G, S>> {
        using target_type = egret::inst::ois_spread_exclusive_compounding<G, S>;

        template <typename Json>
        static target_type from_json(const Json& j)
        {
            namespace j2obj = egret::util::j2obj;
            constexpr auto deser = j2obj::construct<target_type>(
                "gearing" >> j2obj::get<G>,
                "spread" >> j2obj::get<egret::mkt::rate<S>>
            );
            return deser(j);
        }
        template <typename Json>
        static void to_json(Json& j, const target_type& conv)
        {
            j["gearing"] = conv.gearing;
            j["spread"] = conv.spread;
        }
    };

    template <typename S>
    struct adl_serializer<egret::inst::ois_flat_compounding<S>> {
        using target_type = egret::inst::ois_flat_compounding<S>;

        template <typename Json>
        static target_type from_json(const Json& j)
        {
            namespace j2obj = egret::util::j2obj;
            constexpr auto deser = j2obj::construct<target_type>(
                "spread" >> j2obj::get<egret::mkt::rate<S>>
            );
            return deser(j);
        }
        template <typename Json>
        static void to_json(Json& j, const target_type& conv)
        {
            j["spread"] = conv.spread;
        }
    };
    
    template <typename S>
    struct adl_serializer<egret::inst::ois_straight_compounding<S>> {
        using target_type = egret::inst::ois_straight_compounding<S>;

        template <typename Json>
        static target_type from_json(const Json& j)
        {
            namespace j2obj = egret::util::j2obj;
            constexpr auto deser = j2obj::construct<target_type>(
                "spread" >> j2obj::get<egret::mkt::rate<S>>
            );
            return deser(j);
        }
        template <typename Json>
        static void to_json(Json& j, const target_type& conv)
        {
            j["spread"] = conv.spread;
        }
    };
    
    template <typename G, typename S>
    struct adl_serializer<egret::inst::ois_compound_convention<G, S>> {
        using target_type = egret::inst::ois_compound_convention<G, S>;

        using pair_t = std::pair<const char*, std::type_index>;
        static inline const auto string_type_mapping = std::array{
            pair_t {"spread_exclusive", typeid(egret::inst::ois_spread_exclusive_compounding<G, S>)},
            pair_t {"flat", typeid(egret::inst::ois_flat_compounding<S>)},
            pair_t {"straight", typeid(egret::inst::ois_straight_compounding<S>)},
        };

        template <typename Json>
        static target_type from_json(const Json& j)
        {
            namespace j2obj = egret::util::j2obj;
            constexpr auto deser = j2obj::variant(
                [](const Json& j) -> std::type_index {
                    const auto& type = j.at("type").template get_ref<const typename Json::string_t&>();
                    return egret::util::string_to_type(string_type_mapping, type);
                },
                j2obj::get<egret::inst::ois_spread_exclusive_compounding<G, S>>,
                j2obj::get<egret::inst::ois_flat_compounding<S>>,
                j2obj::get<egret::inst::ois_straight_compounding<S>>
            );
            return deser(j);
        }

        template <typename Json>
        static void to_json(Json& j, const target_type& conv)
        {
            egret::util::variant_to_json(j, conv);
            j["type"] = egret::util::type_to_string(string_type_mapping, egret::util::variant_type_index(conv));
        }
    };
    
    template <
        typename RateTag, typename DC, typename Obs, typename Cmp,
        typename N
    >
    struct adl_serializer<egret::inst::ois_coupon_cf<RateTag, DC, Obs, Cmp, N>> {
        template <typename Json>
        static egret::inst::ois_coupon_cf<RateTag, DC, Obs, Cmp, N> from_json(const Json& j)
        {
            namespace util = egret::util;
            namespace j2obj = util::j2obj;
            constexpr auto deser = j2obj::construct<egret::inst::ois_coupon_cf<RateTag, DC, Obs, Cmp, N>>(
                "discount_tag" >> j2obj::get<RateTag>,
                "rate_tag" >> j2obj::get<RateTag>,
                "notional" >> j2obj::get<N>,
                "accrual_start" >> j2obj::string.parse_to<std::chrono::sys_days>("%F"),
                "accrual_end" >> j2obj::string.parse_to<std::chrono::sys_days>("%F"),
                "payment_date" >> j2obj::string.parse_to<std::chrono::sys_days>("%F"),
                "accrual_daycounter" >> j2obj::get<DC>,
                "observation_convention" >> j2obj::get<Obs>,
                "compound_convention" >> j2obj::get<Cmp>
            );
            return deser(j);
        }
        template <typename Json>
        static void to_json(Json& j, const egret::inst::ois_coupon_cf<RateTag, DC, Obs, Cmp, N>& cf)
        {
            namespace util = egret::util;
            j["discount_tag"] = cf.discount_tag;
            j["rate_tag"] = cf.rate_tag;
            j["notional"] = cf.notional;
            j["accrual_start"] = util::to_string(cf.accrual_start);
            j["accrual_end"] = util::to_string(cf.accrual_end);
            j["payment_date"] = util::to_string(cf.accrual_end);
            j["accrual_daycounter"] = cf.accrual_daycounter;
            j["observation_convention"] = cf.observation_convention;
            j["compound_convention"] = cf.compound_convention;
        }
    };

} // namespace nlohmann
