#pragma once

#include <nlohmann/json_fwd.hpp>
#include "core/utils/json_utils/j2obj.h"
#include "egret/models/curves/discount_factor.h"
#include "egret/chrono/daycounters/concepts.h"
#include "egret/instruments/cashflows/fixed_leg.h"
#include "egret/instruments/cashflows/term_rate_leg.h"
#include "swap_constraint.h"
#include "fixed_leg.h"
#include "term_rate_leg.h"

namespace egret::fit::yc {
// -----------------------------------------------------------------------------
//  [type] irs_constraint
// -----------------------------------------------------------------------------
    template <typename DiscountTag, typename FloatingTag, typename DC>
    using irs_constraint = swap_constraint<
        inst::cfs::term_rate_leg_header<DiscountTag, FloatingTag, DC>, inst::cfs::term_rate_cashflow<>,
        inst::cfs::fixed_leg_header<DiscountTag, DC>, inst::cfs::fixed_rate_cf<>
    >;

} // namespace egret::fit::yc

namespace nlohmann {
    template <typename DiscountTag, typename FloaterTag, typename DC>
    struct adl_serializer<egret::fit::yc::irs_constraint<DiscountTag, FloaterTag, DC>> {

        using target_type = egret::fit::yc::irs_constraint<DiscountTag, FloaterTag, DC>;
        using fixed_leg = egret::inst::cfs::fixed_leg<DiscountTag, DC>;
        using floating_leg = egret::inst::cfs::term_rate_leg<DiscountTag, FloaterTag, DC>;

        static constexpr auto decer = egret::util::j2obj::construct<target_type>(
            "floating_leg" >> egret::util::j2obj::get<floating_leg>,
            "fixed_leg" >> egret::util::j2obj::get<fixed_leg>
        );

        template <typename Json>
            requires requires (const Json& j) { deser(j); }
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
            j["fixed_leg"] = obj.receive_leg();
            j["floating_leg"] = obj.pay_leg();
        }
    };

} // namespace nlohmann
