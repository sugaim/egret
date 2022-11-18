#pragma once

#include <chrono>
#include "core/chrono/calendars/calendar.h"
#include "core/chrono/calendars/calendar_server.h"
#include "core/utils/json_utils/j2obj.h"
#include "egret/quantities/rate.h"
#include "leg.h"

namespace egret::inst::cfs {
// -----------------------------------------------------------------------------
//  [struct] overnight_index_leg_header
//  [fn] from_dto
//  [fn] to_dto
// -----------------------------------------------------------------------------
    template <
        typename DiscountTag, typename RateTag, typename DC, 
        typename Cal = chrono::calendar, typename N = double
    >
    struct overnight_index_leg_header {
        DiscountTag discount_curve;
        RateTag projection_curve;
        DC rate_daycounter;
        DC accrual_daycounter;
        Cal rate_reference_calendar = {};
        N notional;
    };

    template <typename DiscountTag, typename RateTag, typename DC, typename N>
    auto from_dto(
        const overnight_index_leg_header<DiscountTag, RateTag, DC, chrono::calendar_identifier, N>& header,
        const chrono::calendar_server& server
    )
        -> overnight_index_leg_header<DiscountTag, RateTag, DC, chrono::calendar, N>
    {
        return {
            header.discount_curve,
            header.projection_curve,
            header.rate_daycounter,
            header.accrual_daycounter,
            server.get(header.rate_reference_calendar),
            header.notional
        };
    }

    template <typename DiscountTag, typename RateTag, typename DC, typename N>
    auto to_dto(const overnight_index_leg_header<DiscountTag, RateTag, DC, chrono::calendar_identifier, N>& header)
        -> overnight_index_leg_header<DiscountTag, RateTag, DC, chrono::calendar_identifier, N>
    {
        return {
            header.discount_curve,
            header.projection_curve,
            header.rate_daycounter,
            header.accrual_daycounter,
            header.rate_reference_calendar.identifier(),
            header.notional
        };

    }

// -----------------------------------------------------------------------------
//  [struct] overnight_index_cashflow
// -----------------------------------------------------------------------------
    template <typename N = double, typename G = double, typename R = G>
    struct overnight_index_cashflow {
        N notional_ratio;
        std::chrono::days lookback = {};
        std::chrono::days backward_shift = {};
        std::chrono::days lockout = {};
        std::chrono::sys_days accrual_start = {};
        std::chrono::sys_days accrual_end = {};
        std::chrono::sys_days fixing_date = {};
        std::chrono::sys_days payment_date = {};
        std::chrono::sys_days cashout_date = {};
        std::chrono::sys_days entitlement_date = {};
        G gearing;
        rate<R> spread;
        std::optional<rate<double>> fixed_coupon_rate;
    };

// -----------------------------------------------------------------------------
//  [type] overnight_index_leg
//  [fn] from_dto
//  [fn] to_dto
// -----------------------------------------------------------------------------
    template <
        typename DiscountTag, typename RateTag, typename DC,
        typename Cal = chrono::calendar, 
        typename N = double, typename NR = N,
        typename G = double, typename R = G
    >
    using overnight_index_leg = leg<
        overnight_index_leg_header<DiscountTag, RateTag, DC, Cal, N>,
        overnight_index_cashflow<NR, G, R>
    >;

    template <
        typename DTag, typename RTag, typename DC,
        typename N, typename NR, typename G, typename R
    >
    overnight_index_leg<DTag, RTag, DC, chrono::calendar, N, NR, G, R> from_dto(
        const overnight_index_leg<DTag, RTag, DC, chrono::calendar_identifier, N, NR, G, R>& leg,
        const chrono::calendar_server& server
    )
    {
        return {
            from_dto(leg.header(), server),
            leg.cashflows()
        };
    }
    template <
        typename DTag, typename RTag, typename DC,
        typename N, typename NR, typename G, typename R
    >
    overnight_index_leg<DTag, RTag, DC, chrono::calendar, N, NR, G, R> from_dto(
        overnight_index_leg<DTag, RTag, DC, chrono::calendar_identifier, N, NR, G, R>&& leg,
        const chrono::calendar_server& server
    )
    {
        return {
            from_dto(leg.header(), server),
            std::move(leg).cashflows()
        };
    }

    template <
        typename DTag, typename RTag, typename DC,
        typename N, typename NR, typename G, typename R
    >
    overnight_index_leg<DTag, RTag, DC, chrono::calendar_identifier, N, NR, G, R> to_dto(
        const overnight_index_leg<DTag, RTag, DC, chrono::calendar, N, NR, G, R>& leg
    )
    {
        return {
            to_dto(leg.header()),
            leg.cashflows()
        };
    }

    template <
        typename DTag, typename RTag, typename DC,
        typename N, typename NR, typename G, typename R
    >
    overnight_index_leg<DTag, RTag, DC, chrono::calendar_identifier, N, NR, G, R> to_dto(
        overnight_index_leg<DTag, RTag, DC, chrono::calendar, N, NR, G, R>&& leg
    )
    {
        return {
            to_dto(leg.header()),
            std::move(leg).cashflows()
        };
    }

} // namespace egret::inst::cfs

namespace nlohmann {
    template <typename DTag, typename RTag, typename DC, typename Cal, typename N>
    struct adl_serializer<egret::inst::cfs::overnight_index_leg_header<DTag, RTag, DC, Cal, N>> {
        using target_type = egret::inst::cfs::overnight_index_leg_header<DTag, RTag, DC, Cal, N>;

        static constexpr auto deser = egret::util::j2obj::construct<target_type>(
            "discount_curve" >> egret::util::j2obj::get<DTag>,
            "projection_curve" >> egret::util::j2obj::get<RTag>,
            "rate_daycounter" >> egret::util::j2obj::get<DC>,
            "accrual_daycounter" >> egret::util::j2obj::get<DC>,
            "rate_reference_calendar" >> egret::util::j2obj::get<Cal>,
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
                std::is_assignable_v<Json&, const DTag&> &&
                std::is_assignable_v<Json&, const RTag&> &&
                std::is_assignable_v<Json&, const DC&> &&
                std::is_assignable_v<Json&, const Cal&> &&
                std::is_assignable_v<Json&, const N&>
        static void to_json(Json& j, const target_type& obj)
        {
            j["discount_curve"] = obj.discount_curve;
            j["projection_curve"] = obj.projection_curve;
            j["rate_daycounter"] = obj.rate_daycounter;
            j["accrual_daycounter"] = obj.accrual_daycounter;
            j["rate_reference_calendar"] = obj.rate_reference_calendar;
            j["notional"] = obj.notional;
        }
    };

    template <typename N, typename G, typename R>
    struct adl_serializer<egret::inst::cfs::overnight_index_cashflow<N, G, R>> {

        using target_type = egret::inst::cfs::overnight_index_cashflow<N, G, R>;

        static constexpr auto deser = egret::util::j2obj::construct<target_type>(
            "notional_ratio" >> egret::util::j2obj::get<N>.optional().value_or(static_cast<N>(1)),
            "lookback" >> egret::util::j2obj::integer.to<std::chrono::days>().optional().value_or(std::chrono::days(0)),
            "backward_shift" >> egret::util::j2obj::integer.to<std::chrono::days>().optional().value_or(std::chrono::days(0)),
            "lockout" >> egret::util::j2obj::integer.to<std::chrono::days>().optional().value_or(std::chrono::days(0)),
            "accrual_start" >> egret::util::j2obj::string.parse_to<std::chrono::sys_days>("%F"),
            "accrual_end" >> egret::util::j2obj::string.parse_to<std::chrono::sys_days>("%F"),
            "fixing_date" >> egret::util::j2obj::string.parse_to<std::chrono::sys_days>("%F"),
            "payment_date" >> egret::util::j2obj::string.parse_to<std::chrono::sys_days>("%F"),
            "cashout_date" >> egret::util::j2obj::string.parse_to<std::chrono::sys_days>("%F"),
            "entitlement_date" >> egret::util::j2obj::string.parse_to<std::chrono::sys_days>("%F"),
            "gearing" >> egret::util::j2obj::get<G>.optional().value_or(static_cast<G>(1)),
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
            if (result.lockout.count() < 0) {
                throw egret::exception(
                    "Invalid overnight_index_cashflow because its lockout parameter is negative. "
                    "[lockout='{}']",
                    result.lockout.count()
                );
            }
            if (result.lookback.count() < 0) {
                throw egret::exception(
                    "Invalid overnight_index_cashflow because its lookback parameter is negative. "
                    "[lockout='{}']",
                    result.lookback.count()
                );
            }
            if (result.backward_shift.count() < 0) {
                throw egret::exception(
                    "Invalid overnight_index_cashflow because its backward_shift parameter is negative. "
                    "[lockout='{}']",
                    result.backward_shift.count()
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
            j["lookback"] = obj.lookback.count();
            j["backward_shift"] = obj.backward_shift.count();
            j["lockout"] = obj.lockout.count();
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
