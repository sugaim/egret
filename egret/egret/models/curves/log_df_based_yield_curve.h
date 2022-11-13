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
//  [class] log_df_based_yield_curve
// -----------------------------------------------------------------------------
    template <typename F>
        requires 
            std::invocable<const F&, const std::chrono::sys_days&> &&
            cpt::fst_ord_differentiable<const F&, const std::chrono::sys_days&>
    class log_df_based_yield_curve {
    private:
        using this_type = log_df_based_yield_curve;
        using spfn_props = special_function_properties<util::member_data<F>>;
        using result_type = std::invoke_result_t<const F&, const std::chrono::sys_days&>;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        log_df_based_yield_curve() = delete;
        constexpr log_df_based_yield_curve(const this_type&)
            requires spfn_props::are_copy_constructible_v = default;
        constexpr log_df_based_yield_curve(this_type&&)
            noexcept(spfn_props::are_nothrow_move_constructible_v)
            requires spfn_props::are_move_constructible_v = default;

        template <typename AF>
            requires std::is_constructible_v<util::member_data<F>, AF>
        explicit constexpr log_df_based_yield_curve(AF&& f)
            noexcept(spfn_props::template is_each_nothrow_constructible_from_v<AF>)
            : f_(std::forward<AF>(f))
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
                return std::move(der) * 365.;
            }
            auto dcf = chrono::dcf(chrono::act365f, from, to);
            auto from_log_df = std::invoke(f_.get(), from);
            auto to_log_df = std::invoke(f_.get(), to);

            return (std::move(to_log_df) - std::move(from_log_df)) / std::move(dcf);
        }

    // -------------------------------------------------------------------------
    //  get
    //
        constexpr const F& log_df_curve() const noexcept { return f_.get(); }

    public:
        util::member_data<F> f_;

    }; // class log_df_based_yield_curve

    template <typename F>
    log_df_based_yield_curve(F) -> log_df_based_yield_curve<F>;

    template <typename F>
    log_df_based_yield_curve(std::reference_wrapper<F>) -> log_df_based_yield_curve<F&>;

} // namespace egret::model
