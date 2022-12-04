#pragma once

#include <nlohmann/json_fwd.hpp>
#include "core/utils/json_utils/j2obj.h"
#include "egret/models/curves/concepts.h"
#include "egret/instruments/cashflows/leg.h"
#include "concepts.h"

namespace egret::fit::yc {
// -----------------------------------------------------------------------------
//  [struct] cashflow_evaluator
// -----------------------------------------------------------------------------
    template <typename Header, typename Cashflow>
    struct cashflow_evaluator {};

// -----------------------------------------------------------------------------
//  [struct] specializable_evaluator<inst::cfs::leg<Header, Cashflow>>
// -----------------------------------------------------------------------------
    template <typename Header, typename Cashflow>
    struct specializable_evaluator<inst::cfs::leg<Header, Cashflow>> {
        template <typename RateTag, cpt::yield_curve Curve>
            requires std::invocable<
                const cashflow_evaluator<Header, Cashflow>&,
                const Header&, const Cashflow&,
                const std::chrono::sys_days&,
                const std::map<RateTag, Curve>&
            >
        auto operator()(const inst::cfs::leg<Header, Cashflow>& leg, const std::chrono::sys_days& vdt, const std::map<RateTag, Curve>& curves) const
            -> std::invoke_result_t<
                const cashflow_evaluator<Header, Cashflow>&,
                const Header&, const Cashflow&,
                const std::chrono::sys_days&,
                const std::map<RateTag, Curve>&
            >
        {
            using result_t = std::invoke_result_t<
                const cashflow_evaluator<Header, Cashflow>&,
                const std::chrono::sys_days&,
                const std::map<RateTag, Curve>&
            >;
            constexpr auto evaluator = cashflow_evaluator<Header, Cashflow> {};
            result_t result = 0;
            for (const auto& cf : leg.cashflows()) {
                result += evaluator(leg.header(), cf, vdt, curves);
            }
            return result;
        }
    };

} // namespace egret::fit::yc
