#pragma once

#include <nlohmann/json_fwd.hpp>
#include "core/utils/json_utils/j2obj.h"
#include "egret/models/curves/concepts.h"
#include "egret/instruments/cashflows/leg.h"

namespace egret::fit::yc {
// -----------------------------------------------------------------------------
//  [struct] cashflow_evaluator
// -----------------------------------------------------------------------------
    template <typename Header, typename Cashflow>
    struct cashflow_evaluator {};

// -----------------------------------------------------------------------------
//  [class] leg_evaluator
// -----------------------------------------------------------------------------
    template <typename Header, typename Cashflow>
    struct leg_evaluator {
    private:
        using this_type = leg_evaluator;
        using leg_type = inst::cfs::leg<Header, Cashflow>;
    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        leg_evaluator() = delete;
        leg_evaluator(const this_type&) = default;
        leg_evaluator(this_type&&) noexcept(std::is_nothrow_move_constructible_v<leg_type>) = default;

        explicit leg_evaluator(const leg_type& leg) : leg_(leg) {}
        explicit leg_evaluator(leg_type&& leg) noexcept(std::is_nothrow_move_constructible_v<leg_type>): leg_(std::move(leg)) {}

        this_type& operator =(const this_type&) = default;
        this_type& operator =(this_type&&) noexcept(std::is_nothrow_move_assignable_v<leg_type>) = default;

    // -------------------------------------------------------------------------
    //  yield_curve_evaluator behavior
    //
        template <typename RateTag, cpt::yield_curve Curve>
            requires std::invocable<
                const cashflow_evaluator<Header, Cashflow>&,
                const Header&, const Cashflow&,
                const std::chrono::sys_days&,
                const std::map<RateTag, Curve>&
            >
        auto evaluate(const std::chrono::sys_days& vdt, const std::map<RateTag, Curve>& curves) const
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
            for (const auto& cf : leg_.cashflows()) {
                result += evaluator(leg_.header(), cf, vdt, curves);
            }
            return result;
        }

    // -------------------------------------------------------------------------
    //  get
    //
        const leg_type& leg() const noexcept { return leg_; }

    private:
        inst::cfs::leg<Header, Cashflow> leg_;

    }; // class leg_evaluator

} // namespace egret::fit::yc

namespace nlohmann {
    template <typename H, typename C>
    struct adl_serializer<egret::fit::yc::leg_evaluator<H, C>> {

        using target_type = egret::fit::yc::leg_evaluator<H, C>;

        static constexpr auto deser = egret::util::j2obj::construct<target_type>(
            egret::util::j2obj::get<egret::inst::cfs::leg<H, C>>
        );

        template <typename Json>
            requires requires (const Json& j) { deser(j); }
        static target_type from_json(const Json& j)
        {
            return deser(j);
        }
        template <typename Json>
            requires std::is_assignable_v<Json&, const std::vector<Component>&>
        static void to_json(Json& j, const target_type& obj)
        {
            j = obj.leg();
        }
    };

} // namespace nlohmann