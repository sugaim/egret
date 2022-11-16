#pragma once

#include "egret/chrono/daycounters/concepts.h"
#include "egret/models/curves/concepts.h"
#include "egret/instruments/cashflows/ois_cf.h"
#include "floating_rate_cf.h"

namespace egret::analytic {
// -----------------------------------------------------------------------------
//  [struct] floating_coupon_rate_calculator
// -----------------------------------------------------------------------------
    template <cpt::yield_curve Curve, typename G, typename S, cpt::daycounter DC>
    struct floating_coupon_rate_calculator<Curve, inst::cfs::ois_definition<
        inst::cfs::ois_spread_exclusive_compounding<G, S>, DC, chrono::calendar>> {

        using comp_conv = inst::cfs::ois_spread_exclusive_compounding<G, S>;
        using rate_def_t = inst::cfs::ois_definition<comp_conv, DC, chrono::calendar>;

        constexpr auto operator()(
            const Curve& crv, 
            const rate_def_t& rate_def,
            const std::chrono::sys_days& acc_stt ,
            const std::chrono::sys_days& acc_end 
        ) const
            -> model::forward_rate_t<Curve>
        {
            const auto& stt = rate_def;
            const auto& end = rate_def.reference_end;
            if (stt == end) {
                return model::forward_rate(pcurve, stt, end);
            }
            else {
                const double dcf = chrono::dcf(rate_def.rate_daycounter, stt, end);
                auto fwd_df = model::discount_factor(crv, stt, end);
                return (1. / std::move(fwd_df) - 1.) / dcf;
            }
        }
    };

} // namespace egret::analytic
