#pragma once

#include <type_traits>
#include <concepts>
#include "core/concepts/qualified.h"
#include "core/utils/member_data.h"
#include "core/math/integrate.h"
#include "core/type_traits/special_functions_properties.h"
#include "egret/chrono/daycounters/concepts.h"

namespace egret::model {
// -----------------------------------------------------------------------------
//  [class] forward_rate_based_yield_curve
// -----------------------------------------------------------------------------
    template <typename F>
        requires 
            std::invocable<const F&, const std::chrono::sys_days&> &&
            cpt::integrable<const F&, const std::chrono::sys_days&>
    class forward_rate_based_yield_curve {
    private:
        using this_type = forward_rate_based_yield_curve;
        using spfn_props = special_function_properties<util::member_data<F>>;
        using result_type = std::invoke_result_t<const F&, const std::chrono::sys_days&>;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        forward_rate_based_yield_curve() = delete;
        constexpr forward_rate_based_yield_curve(const this_type&)
            requires spfn_props::are_copy_constructible_v = default;
        constexpr forward_rate_based_yield_curve(this_type&&)
            noexcept(spfn_props::are_nothrow_move_constructible_v)
            requires spfn_props::are_move_constructible_v = default;

        template <typename AF>
            requires std::is_constructible_v<util::member_data<F>, AF>
        explicit constexpr forward_rate_based_yield_curve(AF&& f)
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
                return std::invoke(f_.get(), from);
            }
            auto integrated = math::integrate(f_.get(), from, to);
            return std::move(integrated) / static_cast<double>((to - from).count());
        }

    // -------------------------------------------------------------------------
    //  get
    //
        constexpr const F& instantaneous_forward_curve() const noexcept { return f_.get(); }

    public:
        util::member_data<F> f_;

    }; // class forward_rate_based_yield_curve

    template <typename F>
    forward_rate_based_yield_curve(F) -> forward_rate_based_yield_curve<F>;

    template <typename F>
    forward_rate_based_yield_curve(std::reference_wrapper<F>) -> forward_rate_based_yield_curve<F&>;

} // namespace egret::model
