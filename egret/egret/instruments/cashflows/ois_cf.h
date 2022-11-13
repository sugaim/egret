#pragma once

#include <chrono>
#include "core/chrono/calendars/calendar.h"
#include "core/chrono/calendars/calendar_server.h"
#include "core/utils/json_utils/j2obj.h"
#include "egret/quantities/rate.h"
#include "floating_rate_cf.h"

namespace egret::inst::cfs {
// -----------------------------------------------------------------------------
//  [struct] ois_spread_exclusive_compounding
// -----------------------------------------------------------------------------
    template <typename G = double, typename S = G>
    struct ois_spread_exclusive_compounding {
        G gearing;
        rate<S> spread;
    };

// -----------------------------------------------------------------------------
//  [struct] ois_flat_compounding
// -----------------------------------------------------------------------------
    template <typename S = double>
    struct ois_flat_compounding {
        rate<S> spread;
    };

// -----------------------------------------------------------------------------
//  [struct] ois_straight_compounding
// -----------------------------------------------------------------------------
    template <typename S = double>
    struct ois_straight_compounding {
        rate<S> spread;
    };

// -----------------------------------------------------------------------------
//  [struct] ois_definition
//  [fn] from_dto
//  [fn] to_dto
// -----------------------------------------------------------------------------
    template <typename CompConv, typename DC, typename Cal = chrono::calendar>
    struct ois_definition {
        CompConv compound_convention;

        std::chrono::days lockout = {};
        std::chrono::days lookback = {};
        std::chrono::days observation_period_shift = {};

        DC rate_daycounter;
        Cal rate_reference_calendar = {};
        Cal compound_weight_calendar = {};
    };

    template <typename CompConv>
    ois_definition<CompConv, chrono::calendar> from_dto(
        const ois_definition<CompConv, chrono::calendar_identifier>& def,
        const chrono::calendar_server& server
    )
    {
        return {            
            def.compound_convention,
            def.lockout,
            def.lookback,
            def.observation_period_shift,
            server.get(def.rate_reference_calendar),
            server.get(def.compound_weight_calendar)
        };
    }
    template <typename CompConv>
    ois_definition<CompConv, chrono::calendar_identifier> to_dto(
        const ois_definition<CompConv, chrono::calendar>& def
    )
    {
        return {            
            def.compound_convention,
            def.lockout,
            def.lookback,
            def.observation_period_shift,
            def.rate_reference_calendar.identifier(),
            def.compound_weight_calendar.identifier()
        };
    }

// -----------------------------------------------------------------------------
//  [type] ois_cf
//  [type] spread_exclusive_ois_cf
//  [type] flat_ois_cf
//  [type] straight_ois_cf
//  [fn] from_dto
//  [fn] to_dto
// -----------------------------------------------------------------------------
    template <
        typename DiscountTag, typename RateTag, typename DC,
        typename CompConv, typename Cal = chrono::calendar,
        typename N = double, typename R = double
    >
    using ois_cf = floating_rate_cf<
        DiscountTag, RateTag, ois_definition<CompConv, Cal>, DC, N, R>;

    template <
        typename DiscountTag, typename RateTag, typename DC,
        typename Cal = chrono::calendar,
        typename N = double, typename G = double, typename S = G, typename R = double
    >
    using spread_exclusive_ois_cf = ois_cf<
        DiscountTag, RateTag, DC,
        ois_spread_exclusive_compounding<G, S>, 
        N, R
    >;

    template <
        typename DiscountTag, typename RateTag, typename DC,
        typename Cal = chrono::calendar,
        typename N = double, typename S = double, typename R = double
    >
    using flat_ois_cf = ois_cf<
        DiscountTag, RateTag, DC, 
        ois_flat_compounding<S>, 
        N, R
    >;
    
    template <
        typename DiscountTag, typename RateTag, typename DC,
        typename Cal = chrono::calendar,
        typename N = double, typename S = double, typename R = double
    >
    using straight_ois_cf = ois_cf<
        DiscountTag, RateTag, DC, 
        ois_straight_compounding<S>, 
        N, R
    >;

    template <
        typename DTag, typename RTag, typename DC,
        typename C, typename N, typename R
    >
    ois_cf<DTag, RTag, DC, C, chrono::calendar, N, R> from_dto(
        const ois_cf<DTag, RTag, DC, C, chrono::calendar_identifier, N, R>& cf,
        const chrono::calendar_server& server
    )
    {
        return {
            cf.discount_tag, cf.curve_tag,
            cfs::from_dto(cf.rate_definition, server),
            cf.notional,
            cf.accrual_start, cf.accrual_end, cf.fixing_date,
            cf.payment_date, cf.cashout_date, cf.accrual_daycounter, cf.fixed_coupon_rate
        };
    }

    template <
        typename DTag, typename RTag, typename DC,
        typename C, typename N, typename R
    >
    ois_cf<DTag, RTag, DC, C, chrono::calendar_identifier, N, R> to_dto(
        const ois_cf<DTag, RTag, DC, C, chrono::calendar, N, R>& cf
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
    template <typename G, typename S>
    struct adl_serializer<egret::inst::cfs::ois_spread_exclusive_compounding<G, S>> {
        using target_type = egret::inst::cfs::ois_spread_exclusive_compounding<G, S>;

        static constexpr auto deser = egret::util::j2obj::construct<target_type>(
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
                std::is_assignable_v<Json&, const G&> &&
                std::is_assignable_v<Json&, const egret::rate<S>&>
        static void to_json(Json& j, const target_type& conv)
        {
            j["gearing"] = conv.gearing;
            j["spread"] = conv.spread;
        }
    };

    template <typename S>
    struct adl_serializer<egret::inst::cfs::ois_flat_compounding<S>> {
        using target_type = egret::inst::cfs::ois_flat_compounding<S>;

        static constexpr auto deser = egret::util::j2obj::construct<target_type>(
            "spread" >> egret::util::j2obj::get<egret::rate<S>>
        );
        template <typename Json>
            requires requires (const Json& j) { deser(j); }
        static target_type from_json(const Json& j)
        {
            return deser(j);
        }
        template <typename Json>
            requires std::is_assignable_v<Json&, const egret::rate<S>&>
        static void to_json(Json& j, const target_type& conv)
        {
            j["spread"] = conv.spread;
        }
    };
    
    template <typename S>
    struct adl_serializer<egret::inst::cfs::ois_straight_compounding<S>> {
        using target_type = egret::inst::cfs::ois_straight_compounding<S>;

        static constexpr auto deser = egret::util::j2obj::construct<target_type>(
            "spread" >> egret::util::j2obj::get<egret::rate<S>>
        );
        template <typename Json>
            requires requires (const Json& j) { deser(j); }
        static target_type from_json(const Json& j)
        {
            return deser(j);
        }
        template <typename Json>
            requires std::is_assignable_v<Json&, const egret::rate<S>&>
        static void to_json(Json& j, const target_type& conv)
        {
            j["spread"] = conv.spread;
        }
    };
    
    template <typename CompConv, typename Cal>
    struct adl_serializer<egret::inst::cfs::ois_definition<CompConv, Cal>> {
    
        using target_type = egret::inst::cfs::ois_definition<CompConv, Cal>;

        static constexpr auto deser = egret::util::j2obj::construct<target_type>(
            "compound_convention" >> egret::util::j2obj::get<CompConv>,
            "lockout" >> egret::util::j2obj::integer.to<std::chrono::days>(),
            "lookback" >> egret::util::j2obj::integer.to<std::chrono::days>(),
            "observation_period_shift" >> egret::util::j2obj::integer.to<std::chrono::days>(),
            "rate_reference_calendar" >> egret::util::j2obj::get<Cal>,
            "compound_weight_calendar" >> egret::util::j2obj::get<Cal>
        );
        template <typename Json>
            requires requires (const Json& j) { deser(j); }
        static target_type from_json(const Json& j)
        {
            return deser(j);
        }
        template <typename Json>
            requires
                std::is_assignable_v<Json&, const CompConv&> &&
                std::is_assignable_v<Json&, const Cal&>
        static void to_json(Json& j, const target_type& cf)
        {
            j["compound"] = cf.compound_convention;
            j["lockout"] = cf.lockout.count();
            j["lookback"] = cf.lookback.count();
            j["observation_period_shift"] = cf.observation_period_shift();
            j["rate_reference_calendar"] = cf.rate_reference_calendar;
            j["compound_weight_calendar"] = cf.compound_weight_calendar;
        }
    };

} // namespace nlohmann
