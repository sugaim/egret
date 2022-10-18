#pragma once

#include <chrono>
#include <algorithm>
#include "core/type_traits/special_functions_properties.h"
#include "core/utils/member_data.h"
#include "concepts.h"

namespace egret::model {
// -----------------------------------------------------------------------------
//  [class] joint_yield_curve
// -----------------------------------------------------------------------------
    template <cpt::yield_curve ShortTerm, cpt::yield_curve LongTerm>
    class joint_yield_curve {
    private:
        using this_type = joint_yield_curve;
        using spfn_props = special_function_properties<
            util::member_data<ShortTerm>, util::member_data<LongTerm>, std::chrono::sys_days>;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        joint_yield_curve() = delete;
        constexpr joint_yield_curve(const this_type&)
            noexcept(spfn_props::are_nothrow_copy_constructible_v)
            requires spfn_props::are_copy_constructible_v = default;
        constexpr joint_yield_curve(this_type&&)
            noexcept(spfn_props::are_nothrow_move_constructible_v)
            requires spfn_props::are_move_constructible_v = default;

        template <typename S, typename L>
            requires 
                std::is_constructible_v<util::member_data<ShortTerm>, S> &&
                std::is_constructible_v<util::member_data<LongTerm>, L>
        constexpr joint_yield_curve(S&& short_term, L&& long_term, const std::chrono::sys_days& separation)
            noexcept(spfn_props::template is_each_nothrow_constructible_from_v<S, L, const std::chrono::sys_days&>)
            : short_term_(std::forward<S>(short_term)),
              long_term_(std::forward<L>(long_term)),
              separation_(separation)
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
        constexpr auto forward_rate(const std::chrono::sys_days& from, const std::chrono::sys_days& to) const
            -> std::common_type_t<forward_rate_t<ShortTerm>, forward_rate_t<LongTerm>>
        {
            using result_t = std::common_type_t<forward_rate_t<ShortTerm>, forward_rate_t<LongTerm>>;
            const auto& [f, t] = std::minmax(from, to);
            if (separation_ <= f /* <= t */) [[likely]] {
                return static_cast<result_t>(model::forward_rate(long_term_.get(), f, t));
            }
            else if (/* f <= */ t <= separation_) {
                return static_cast<result_t>(model::forward_rate(short_term_.get(), f, t));
            }
            else [[unlikely]] {
                // f < separation_ < t
                auto s_fwd = model::forward_rate(short_term_.get(), f, separation_);
                auto l_fwd = model::forward_rate(long_term_.get(), separation_, t);
                const auto s_term     = static_cast<double>((separation_ - f).count());
                const auto l_term     = static_cast<double>((t - separation_).count());
                const auto whole_term = static_cast<double>((t - f).count());

                return (
                    static_cast<result_t>(std::move(s_fwd)) * s_term + 
                    static_cast<result_t>(std::move(l_fwd)) * l_term
                ) / whole_term;
            }
        }

    // -------------------------------------------------------------------------
    //  get
    //
        constexpr const ShortTerm& short_term_curve() const noexcept { return short_term_.get(); }
        constexpr const LongTerm& long_term_curve() const noexcept { return long_term_.get(); }
        constexpr const std::chrono::sys_days& separation() const noexcept { return separation_; }

    public:
        util::member_data<ShortTerm> short_term_;
        util::member_data<LongTerm> long_term_;
        std::chrono::sys_days separation_;

    }; // class joint_yield_curve
    
    template <typename S, typename L>
    joint_yield_curve(S, L, std::chrono::sys_days) -> joint_yield_curve<S, L>;

    template <typename S, typename L>
    joint_yield_curve(std::reference_wrapper<S>, L, std::chrono::sys_days) -> joint_yield_curve<S&, L>;

    template <typename S, typename L>
    joint_yield_curve(S, std::reference_wrapper<L>, std::chrono::sys_days) -> joint_yield_curve<S, L&>;

    template <typename S, typename L>
    joint_yield_curve(std::reference_wrapper<S>, std::reference_wrapper<L>, std::chrono::sys_days) -> joint_yield_curve<S&, L&>;

} // namespace egret::model
