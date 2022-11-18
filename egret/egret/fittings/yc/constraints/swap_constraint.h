#pragma once

#include "concepts.h"
#include "leg_evaluator.h"

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
            noexcept(std::is_nothrow_move_constructible_v<leg_evaluator<PH, PC>> &&
                     std::is_nothrow_move_constructible_v<leg_evaluator<RH, RC>>) = default;

        swap_constraint(payleg_type pay_leg, receiveleg_type rec_leg)
            noexcept(std::is_nothrow_move_constructible_v<leg_evaluator<PH, PC>> &&
                     std::is_nothrow_move_constructible_v<leg_evaluator<RH, RC>>)
            : pay_(std::move(pay_leg)), rec_(std::move(rec_leg))
        {
        }

        swap_constraint(leg_evaluator<PH, PC> pay_leg, leg_evaluator<RH, RC> rec_leg)
            noexcept(std::is_nothrow_move_constructible_v<leg_evaluator<PH, PC>> &&
                     std::is_nothrow_move_constructible_v<leg_evaluator<RH, RC>>)
            : pay_(std::move(pay_leg)), rec_(std::move(rec_leg))
        {
        }

        this_type& operator =(const this_type&) = default;
        this_type& operator =(this_type&&)
            noexcept(std::is_nothrow_move_assignable_v<leg_evaluator<PH, PC>> &&
                     std::is_nothrow_move_assignable_v<leg_evaluator<RH, RC>>) = default;

    // -------------------------------------------------------------------------
    //  yield_curve_evaluator behavior
    //
        template <typename RateTag, cpt::yield_curve Curve>
            requires 
                cpt::yield_curve_evaluator<leg_evaluator<PH, PC>, RateTag, Curve> &&
                cpt::yield_curve_evaluator<leg_evaluator<RH, RC>, RateTag, Curve>
        auto evaluate(const std::chrono::sys_days& vdt, const std::map<RateTag, Curve>& curves) const
        {
            auto rec = rec_.evaluate(vdt, curves);
            auto pay = pay_.evaluate(vdt, curves);
            return std::move(rec) - std::move(pay);
        }

    // -------------------------------------------------------------------------
    //  get
    //
        const payleg_type& pay_leg() const noexcept { return pay_.leg(); }
        const receiveleg_type& receive_leg() const noexcept { return rec_.leg(); }

    private:
        leg_evaluator<PH, PC> pay_;
        leg_evaluator<RH, RC> rec_;

    }; // class swap_constraint

} // namespace egret::fit::yc