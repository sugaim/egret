#pragma once

#include <nlohmann/json_fwd.hpp>
#include "core/utils/json_utils/j2obj.h"
#include "swap_constraint.h"
#include "egret/analytic/cashflows/fixed_rate_cf.h"
#include "egret/analytic/cashflows/term_rate_cf.h"

namespace egret::fit::yc {
// -----------------------------------------------------------------------------
//  [type] fixed_floating_swap
// -----------------------------------------------------------------------------
    template <typename DiscountTag, typename FloaterTag, typename DC>
    using fixed_floating_swap = swap_constraint<
        inst::cfs::fixed_rate_cf<DiscountTag, DC>,
        inst::cfs::term_rate_cf<DiscountTag, FloaterTag, DC>
    >;

} // namespace egret::fit::yc

namespace nlohmann {
    template <typename DiscountTag, typename FloaterTag, typename DC>
    struct adl_serializer<egret::fit::yc::fixed_floating_swap<DiscountTag, FloaterTag, DC>> {

        using target_type = egret::fit::yc::fixed_floating_swap<DiscountTag, FloaterTag, DC>;
        using fixed_cf = egret::inst::cfs::fixed_rate_cf<DiscountTag, DC>;
        using floating_cf = egret::inst::cfs::term_rate_cf<DiscountTag, FloaterTag, DC>;

        static constexpr auto decer = egret::util::j2obj::construct<target_type>(
            "fixed_leg" >> egret::util::j2obj::array.to_vector_of<fixed_cf>(),
            "floating_leg" >> egret::util::j2obj::array.to_vector_of<floating_cf>()
        );

        template <typename Json>
            requires requires (const Json& j) { deser(j); }
        static target_type from_json(const Json& j)
        {
            return decer(j);
        }

        template <typename Json>
            requires 
                std::is_assignable_v<Json&, const std::vector<fixed_cf>&> &&
                std::is_assignable_v<Json&, const std::vector<floating_cf>&>
        static void to_json(Json& j, const target_type& obj)
        {
            j["fixed_leg"] = obj.receive_leg();
            j["floating_leg"] = obj.payment_leg();
        }
    };

} // namespace nlohmann
