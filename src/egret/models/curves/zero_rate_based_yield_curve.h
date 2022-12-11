#pragma once

#include <type_traits>
#include <concepts>
#include "core/concepts/qualified.h"
#include "core/math/der.h"
#include "core/utils/member_data.h"
#include "core/type_traits/special_functions_properties.h"
#include "egret/chrono/daycounters/concepts.h"
#include "egret/chrono/daycounters/act365f.h"

namespace egret::model {
// -----------------------------------------------------------------------------
//  [class] zero_rate_based_yield_curve
// -----------------------------------------------------------------------------
    template <typename F>
        requires 
            std::invocable<const F&, const std::chrono::sys_days&> &&
            cpt::fst_ord_differentiable<const F&, const std::chrono::sys_days&>
    class zero_rate_based_yield_curve {
    private:
        using this_type = zero_rate_based_yield_curve;
        using spfn_props = special_function_properties<util::member_data<F>, std::chrono::sys_days>;
        using result_type = std::invoke_result_t<const F&, const std::chrono::sys_days&>;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        zero_rate_based_yield_curve() = delete;
        constexpr zero_rate_based_yield_curve(const this_type&)
            requires spfn_props::are_copy_constructible_v = default;
        constexpr zero_rate_based_yield_curve(this_type&&)
            noexcept(spfn_props::are_nothrow_move_constructible_v)
            requires spfn_props::are_move_constructible_v = default;

        template <typename AF>
            requires std::is_constructible_v<util::member_data<F>, AF>
        constexpr zero_rate_based_yield_curve(AF&& f, const std::chrono::sys_days& quote_date)
            noexcept(spfn_props::template is_each_nothrow_constructible_from_v<AF, const std::chrono::sys_days&>)
            : f_(std::forward<AF>(f)), qdt_(quote_date)
        {
        }

        constexpr this_type& operator =(const this_type&)
            noexcept(spfn_props::are_nothrow_copy_assignable_v)
            requires spfn_props::are_copy_assignable_v = default;
        constexpr this_type& operator =(this_type&&)
            noexcept(spfn_props::are_nothrow_move_assignable_v)
            requires spfn_props::are_move_assignable_v = default;

    // -------------------------------------------------------------------------
    //  yield_curve behavior
    //
        constexpr result_type forward_rate(const std::chrono::sys_days& from, const std::chrono::sys_days& to) const
        {
            if (from == to) [[unlikely]] {
                auto der = math::der1(f_.get(), from);
                auto val = std::invoke(f_.get(), from);
                return std::move(val) + std::move(der) * static_cast<double>((from - qdt_).count());
            }
            const auto q2from  = static_cast<double>((from - qdt_).count());
            const auto q2to    = static_cast<double>((to - qdt_).count());
            const auto from2to = static_cast<double>((to - from).count());
            auto from_zero_rate = std::invoke(f_.get(), from);
            auto to_zero_rate = std::invoke(f_.get(), to);

            return (std::move(to_zero_rate) * q2to - std::move(from_zero_rate) * q2from) / from2to;
        }

    // -------------------------------------------------------------------------
    //  get
    //
        constexpr const F& zero_rate_curve() const noexcept { return f_.get(); }
        constexpr const std::chrono::sys_days& quote_date() const noexcept { return qdt_; }

    public:
        util::member_data<F> f_;
        std::chrono::sys_days qdt_;

    }; // class zero_rate_based_yield_curve

    template <typename F>
    zero_rate_based_yield_curve(F, std::chrono::sys_days) -> zero_rate_based_yield_curve<F>;

    template <typename F>
    zero_rate_based_yield_curve(std::reference_wrapper<F>, std::chrono::sys_days) -> zero_rate_based_yield_curve<F&>;

} // namespace egret::model
