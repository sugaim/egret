#pragma once

#include <chrono>
#include "core/chrono/calendars/calendar.h"
#include "core/chrono/calendars/calendar_server.h"
#include "core/utils/json_utils/j2obj.h"
#include "egret/quantities/rate.h"
#include "floating_rate_cf.h"

namespace egret::inst::cfs {
// -----------------------------------------------------------------------------
//  [struct] overnight_index_definition
//  [fn] from_dto
//  [fn] to_dto
// -----------------------------------------------------------------------------    
    /**
     * @brief Definition of overnight index (spread exclusive compounding).
     * 
     * @tparam DC 
     * @tparam Cal 
     * @tparam G 
     * @tparam S 
    */
    template <typename DC, typename Cal = chrono::calendar, typename G = double, typename S = G>
    struct overnight_index_definition {
        DC rate_daycounter;
        Cal rate_reference_calendar = {};

        std::chrono::days lookback = {};
        std::chrono::days observation_period_shift = {};
        std::chrono::days lockout = {};

        G gearing = {};
        rate<S> spread = {};
    };

    template <typename DC, typename G, typename S = G>
    auto from_dto(
        const overnight_index_definition<DC, chrono::calendar_identifier, G, S>& def,
        const chrono::calendar_server& server
    )
        -> overnight_index_definition<DC, chrono::calendar, G, S>
    {
        return {
            def.rate_daycounter,
            server.get(def.rate_reference_calendar),
            def.lookback,
            def.observation_period_shift,
            def.lockout,
            def.gearing,
            def.spread
        };
    }

    template <typename DC, typename G, typename S = G>
    auto to_dto(const overnight_index_definition<DC, chrono::calendar, G, S>& def)
        -> overnight_index_definition<DC, chrono::calendar_identifier, G, S>
    {
        return {
            def.rate_daycounter,
            def.rate_reference_calendar.identifier(),
            def.lookback,
            def.observation_period_shift,
            def.lockout,
            def.gearing,
            def.spread
        };
    }

// -----------------------------------------------------------------------------
//  [type] overnight_index_cf
//  [fn] from_dto
//  [fn] to_dto
// -----------------------------------------------------------------------------
    template <
        typename DiscountTag, typename RateTag, typename DC,
        typename Cal = chrono::calendar, typename G = double, typename S = G,
        typename N = double
    >
    using overnight_index_cf = floating_rate_cf<
        DiscountTag, RateTag, overnight_index_definition<DC, Cal, G, S>, DC, N>;

    template <
        typename DTag, typename RTag, typename DC,
        typename G, typename S, typename N
    >
    overnight_index_cf<DTag, RTag, DC, chrono::calendar, G, S, N> from_dto(
        const overnight_index_cf<DTag, RTag, DC, chrono::calendar_identifier, G, S, N>& cf,
        const chrono::calendar_server& server
    )
    {
        return {
            cf.discount_tag, cf.curve_tag,
            from_dto(cf.rate_definition, server),
            cf.notional,
            cf.accrual_start, cf.accrual_end, cf.fixing_date,
            cf.payment_date, cf.cashout_date, cf.accrual_daycounter, cf.fixed_coupon_rate
        };
    }

    template <
        typename DTag, typename RTag, typename DC,
        typename G, typename S, typename N
    >
    overnight_index_cf<DTag, RTag, DC, chrono::calendar_identifier, G, S, N> to_dto(
        const overnight_index_cf<DTag, RTag, DC, chrono::calendar, G, S, N>& cf
    )
    {
        return {
            cf.discount_tag, cf.curve_tag,
            cfs::to_dto(cf.rate_definition),
            cf.notional,
            cf.accrual_start, cf.accrual_end, cf.fixing_date,
            cf.payment_date, cf.cashout_date, cf.accrual_daycounter, cf.fixed_coupon_rate
        };
    }

} // namespace egret::inst::cfs

namespace nlohmann {
    template <typename DC, typename Cal, typename G, typename S>
    struct adl_serializer<egret::inst::cfs::overnight_index_definition<DC, Cal, G, S>> {
        using target_type = egret::inst::cfs::overnight_index_definition<DC, Cal, G, S>;

        static constexpr auto deser = egret::util::j2obj::construct<target_type>(
            "rate_daycounter" >> egret::util::j2obj::get<DC>,
            "rate_reference_calendar" >> egret::util::j2obj::get<Cal>,
            "lookback" >> egret::util::j2obj::integer.to<std::chrono::days>(),
            "observation_period_shift" >> egret::util::j2obj::integer.to<std::chrono::days>(),
            "lockout" >> egret::util::j2obj::integer.to<std::chrono::days>(),
            "gearing" >> egret::util::j2obj::get<G>,
            "spread" >> egret::util::j2obj::get<egret::rate<S>>
        );

        template <typename Json>
            requires requires (const Json& j) { deser(j); }
        static target_type from_json(const Json& j)
        {
            return deser(j);
        }
        template <typename Json>
            requires
                std::is_assignable_v<Json&, const DC&> &&
                std::is_assignable_v<Json&, const Cal&> &&
                std::is_assignable_v<Json&, const G&> &&
                std::is_assignable_v<Json&, const egret::rate<S>&>
        static void to_json(Json& j, const target_type& obj)
        {
            j["rate_daycounter"] = obj.rate_daycounter;
            j["rate_reference_calendar"] = obj.rate_reference_calendar;
            j["lookback"] = obj.lookback.count();
            j["observation_period_shift"] = obj.observation_period_shift.count();
            j["lockout"] = obj.lockout.count();
            j["gearing"] = obj.gearing;
            j["spread"] = obj.spread;
        }
    };

} // namespace nlohmann
