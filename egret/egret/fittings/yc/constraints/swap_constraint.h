#pragma once

#include "concepts.h"
#include "egret/instruments/cashflows/leg.h"

namespace egret::fit::yc {
// -----------------------------------------------------------------------------
//  [class] swap_constraint
// -----------------------------------------------------------------------------
    template <typename PH, typename PC, typename RH, typename RC>
    class swap_constraint {
    private:
        using this_type = swap_constraint;

    public:
        using payleg_type = inst::cfs::leg<PH, PC>;
        using receiveleg_type = inst::cfs::leg<RH, RC>;

    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        swap_constraint() = delete;
        swap_constraint(const this_type&) = default;
        swap_constraint(this_type&&) 
            noexcept(std::is_nothrow_move_constructible_v<payleg_type> &&
                     std::is_nothrow_move_constructible_v<receiveleg_type>) = default;

        swap_constraint(payleg_type pay_leg, receiveleg_type rec_leg)
            noexcept(std::is_nothrow_move_constructible_v<payleg_type> &&
                     std::is_nothrow_move_constructible_v<receiveleg_type>)
            : pay_(std::move(pay_leg)), rec_(std::move(rec_leg))
        {
        }

        this_type& operator =(const this_type&) = default;
        this_type& operator =(this_type&&)
            noexcept(std::is_nothrow_move_assignable_v<payleg_type> &&
                     std::is_nothrow_move_assignable_v<receiveleg_type>) = default;

    // -------------------------------------------------------------------------
    //  yield_curve_evaluator behavior
    //
        template <typename RateTag, cpt::yield_curve Curve>
            requires 
                cpt::yield_curve_evaluator<payleg_type, RateTag, Curve> &&
                cpt::yield_curve_evaluator<receiveleg_type, RateTag, Curve>
        auto evaluate(const std::chrono::sys_days& vdt, const std::map<RateTag, Curve>& curves) const
        {
            auto rec = yc::evaluate(rec_, vdt, curves);
            auto pay = yc::evaluate(pay_, vdt, curves);
            return std::move(rec) - std::move(pay);
        }

    // -------------------------------------------------------------------------
    //  get
    //
        const payleg_type& pay_leg() const & noexcept { return pay_; }
        payleg_type&& pay_leg() && noexcept { return std::move(pay_); }

        const receiveleg_type& receive_leg() const & noexcept { return rec_; }
        receiveleg_type&& receive_leg() && noexcept { return std::move(rec_); }

    private:
        payleg_type pay_;
        receiveleg_type rec_;

    }; // class swap_constraint

} // namespace egret::fit::yc