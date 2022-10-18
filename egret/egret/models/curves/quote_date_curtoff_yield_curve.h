#pragma once

#include <chrono>
#include <algorithm>
#include "core/type_traits/special_functions_properties.h"
#include "core/utils/member_data.h"
#include "concepts.h"

namespace egret::model {
// -----------------------------------------------------------------------------
//  [class] quote_date_cutoff_yield_curve
// -----------------------------------------------------------------------------
    template <cpt::yield_curve C>
    class quote_date_cutoff_yield_curve {
    private:
        using this_type = quote_date_cutoff_yield_curve;
        using spfn_props = special_function_properties<util::member_data<C>, std::chrono::sys_days>;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        quote_date_cutoff_yield_curve() = delete;
        constexpr quote_date_cutoff_yield_curve(const this_type&)
            noexcept(spfn_props::are_nothrow_copy_constructible_v)
            requires spfn_props::are_copy_constructible_v = default;
        constexpr quote_date_cutoff_yield_curve(this_type&&)
            noexcept(spfn_props::are_nothrow_move_constructible_v)
            requires spfn_props::are_move_constructible_v = default;

        template <typename AC>
            requires std::is_constructible_v<util::member_data<C>, AC>
        constexpr quote_date_cutoff_yield_curve(AC&& c, const std::chrono::sys_days& quote_date)
            noexcept(spfn_props::template is_each_nothrow_constructible_from_v<AC, const std::chrono::sys_days&>)
            : c_(std::forward<AC>(c)), qdt_(quote_date)
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
        constexpr forward_rate_t<C> forward_rate(const std::chrono::sys_days& from, const std::chrono::sys_days& to) const
        {
            const auto& [f, t] = std::minmax(from, to);
            if (qdt_ <= f) [[likely]] {
                return model::forward_rate(c_.get(), f, t);
            }
            else if (t < qdt_) [[unlikely]] {
                return static_cast<forward_rate_t<C>>(0);
            }
            else if (f == t) [[unlikely]] {
                return model::forward_rate(c_.get(), f, t);
            }
            const auto q2t = static_cast<double>((t - qdt_).count());
            const auto f2t = static_cast<double>((t - f).count());
            auto fwd = model::forward_rate(c_.get(), qdt_, t);

            return std::move(fwd) * (q2t / f2t);
        }

    // -------------------------------------------------------------------------
    //  get
    //
        constexpr const C& underlying_curve() const noexcept { return c_.get(); }
        constexpr const std::chrono::sys_days& quote_date() const noexcept { return qdt_; }

    public:
        util::member_data<C> c_;
        std::chrono::sys_days qdt_;

    }; // class quote_date_cutoff_yield_curve
    
    template <typename C>
    quote_date_cutoff_yield_curve(C, std::chrono::sys_days) -> quote_date_cutoff_yield_curve<C>;

    template <typename C>
    quote_date_cutoff_yield_curve(std::reference_wrapper<C>, std::chrono::sys_days) -> quote_date_cutoff_yield_curve<C&>;

} // namespace egret::model