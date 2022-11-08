#pragma once

#include "composite_evaluator.h"

namespace egret::fit::yc {
// -----------------------------------------------------------------------------
//  [class] swap_constraint
// -----------------------------------------------------------------------------
    template <typename RecCf, typename PayCf>
    class swap_constraint {
    private:
        using this_type = swap_constraint;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        swap_constraint() = delete;
        swap_constraint(const this_type&) = default;
        swap_constraint(this_type&&) noexcept = default;

        swap_constraint(std::vector<RecCf> receive_leg, std::vector<PayCf> pay_leg) noexcept
            : rec_(std::move(receive_leg)), pay_(std::move(pay_leg))
        {
        }

        this_type& operator =(const this_type&) = default;
        this_type& operator =(this_type&&) noexcept = default;

    // -------------------------------------------------------------------------
    //  yield_curve_evaluator behavior
    //
        template <typename RateTag, cpt::yield_curve Curve>
            requires 
                cpt::yield_curve_evaluator<composite_evaluator<RecCf>, RateTag, Curve> &&
                cpt::yield_curve_evaluator<composite_evaluator<PayCf>, RateTag, Curve>
        auto evaluate(const std::chrono::sys_days& vdt, const std::map<RateTag, Curve>& curves) const
        {
            auto rec = rec_.evaluate(vdt, curves);
            auto pay = pay_.evaluate(vdt, curves);
            return std::move(rec) - std::move(pay);
        }

    // -------------------------------------------------------------------------
    //  get
    //
        const auto& receive_leg() const noexcept { return rec_.components(); }
        const auto& payment_leg() const noexcept { return pay_.components(); }

    private:
        composite_evaluator<RecCf> rec_;
        composite_evaluator<PayCf> pay_;

    }; // class swap_constraint

} // namespace egret::fit::yc