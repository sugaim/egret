#pragma once

#include <nlohmann/json_fwd.hpp>
#include "core/utils/json_utils/j2obj.h"
#include "egret/models/curves/discount_factor.h"
#include "egret/chrono/daycounters/concepts.h"
#include "egret/instruments/cashflows/fixed_leg.h"
#include "egret/instruments/cashflows/overnight_index_leg.h"
#include "swap_constraint.h"
#include "fixed_leg.h"
#include "term_rate_leg.h"

namespace egret::fit::yc {
// -----------------------------------------------------------------------------
//  [type] ois_constraint
// -----------------------------------------------------------------------------
    template <typename DiscountTag, typename FloatingTag, typename DC, typename Cal = chrono::calendar>
    using ois_constraint = swap_constraint<
        inst::cfs::overnight_index_leg_header<DiscountTag, FloatingTag, DC, Cal>, inst::cfs::overnight_index_cashflow<>,
        inst::cfs::fixed_leg_header<DiscountTag, DC>, inst::cfs::fixed_rate_cf<>
    >;

// -----------------------------------------------------------------------------
//  [fn] from_dto
//  [fn] to_dto
// -----------------------------------------------------------------------------
    template <typename DiscountTag, typename FloatingTag, typename DC>
    ois_constraint<DiscountTag, FloatingTag, DC> from_dto(
        const ois_constraint<DiscountTag, FloatingTag, DC, chrono::calendar_identifier>& dto,
        const chrono::calendar_server& server
    )
    {
        return {inst::cfs::from_dto(dto.pay_leg(), server), dto.receive_leg()};
    }

    template <typename DiscountTag, typename FloatingTag, typename DC>
    ois_constraint<DiscountTag, FloatingTag, DC> from_dto(
        ois_constraint<DiscountTag, FloatingTag, DC, chrono::calendar_identifier>&& dto,
        const chrono::calendar_server& server
    )
    {
        return {inst::cfs::from_dto(std::move(dto).pay_leg(), server), std::move(dto).receive_leg()};
    }
    
    template <typename DiscountTag, typename FloatingTag, typename DC>
    ois_constraint<DiscountTag, FloatingTag, DC, chrono::calendar_identifier> to_dto(
        const ois_constraint<DiscountTag, FloatingTag, DC>& obj
    )
    {
        return {inst::cfs::to_dto(obj.pay_leg()), obj.receive_leg()};
    }

    template <typename DiscountTag, typename FloatingTag, typename DC>
    ois_constraint<DiscountTag, FloatingTag, DC, chrono::calendar_identifier> to_dto(
        ois_constraint<DiscountTag, FloatingTag, DC>&& obj
    )
    {
        return {inst::cfs::to_dto(std::move(obj).pay_leg()), std::move(obj).receive_leg()};
    }
    
} // namespace egret::fit::yc

namespace nlohmann {
    template <typename DiscountTag, typename FloaterTag, typename DC, typename Cal>
    struct adl_serializer<egret::fit::yc::ois_constraint<DiscountTag, FloaterTag, DC, Cal>> {

        using target_type = egret::fit::yc::ois_constraint<DiscountTag, FloaterTag, DC, Cal>;
        using fixed_leg = target_type::receiveleg_type;
        using floating_leg = target_type::payleg_type;

        static constexpr auto decer = egret::util::j2obj::construct<target_type>(
            "overnight_index_leg" >> egret::util::j2obj::get<floating_leg>,
            "fixed_leg" >> egret::util::j2obj::get<fixed_leg>
        );

        template <typename Json>
            //requires requires (const Json& j) { deser(j); }
        static target_type from_json(const Json& j)
        {
            return decer(j);
        }

        template <typename Json>
            requires 
                std::is_assignable_v<Json&, const fixed_leg&> &&
                std::is_assignable_v<Json&, const floating_leg&>
        static void to_json(Json& j, const target_type& obj)
        {
            j["overnight_index_leg"] = obj.pay_leg();
            j["fixed_leg"] = obj.receive_leg();
        }
    };

} // namespace nlohmann
