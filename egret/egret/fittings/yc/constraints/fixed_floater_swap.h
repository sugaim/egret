#pragma once

#include <nlohmann/json_fwd.hpp>
#include "core/utils/json_utils/j2obj.h"
#include "swap_constraint.h"
#include "fixed_rate_cf.h"
#include "term_rate_cf.h"

namespace egret::fit::yc {
// -----------------------------------------------------------------------------
//  [type] fixed_floater_swap
// -----------------------------------------------------------------------------
    template <typename DiscountTag, typename FloaterTag, typename DC>
    using fixed_floater_swap = swap_constraint<
        inst::cfs::fixed_rate_cf<DiscountTag, DC>,
        inst::cfs::term_rate_cf<DiscountTag, FloaterTag, DC>
    >;

} // namespace egret::fit::yc

namespace nlohmann {
    template <typename DiscountTag, typename FloaterTag, typename DC>
    struct adl_serializer<egret::fit::yc::fixed_floater_swap<DiscountTag, FloaterTag, DC>> {

        using target_type = egret::fit::yc::fixed_floater_swap<DiscountTag, FloaterTag, DC>;
        using fixed_cf = egret::inst::cfs::fixed_rate_cf<DiscountTag, DC>;
        using floater_cf = egret::inst::cfs::term_rate_cf<DiscountTag, FloaterTag, DC>;

        template <typename Json>
        static target_type from_json(const Json& j)
        {
            namespace j2obj = egret::util::j2obj;
            constexpr auto decer = j2obj::construct<target_type>(
                "fixed_leg" >> j2obj::array.to_vector_of<fixed_cf>(),
                "floater_leg" >> j2obj::array.to_vector_of<floater_cf>()
            );
            return decer(j);
        }

        template <typename Json>
        static void to_json(Json& j, const target_type& obj)
        {
            j["fixed_leg"] = obj.receive_leg();
            j["floater_leg"] = obj.payment_leg();
        }
    };

} // namespace nlohmann
