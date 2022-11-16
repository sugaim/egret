#pragma once

#include <chrono>
#include "adjustment_interface.h"

namespace egret::chrono {
// -----------------------------------------------------------------------------
//  [class] roll
// -----------------------------------------------------------------------------
    class roll : public adjustment_interface<roll> {
    private:
        using this_type = roll;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        roll() = delete;
        constexpr roll(const this_type&) noexcept = default;
        constexpr roll(this_type&&) noexcept = default;

        constexpr roll(std::chrono::day day, std::int32_t next, bool modify_to_eom) noexcept
            : day_(day), next_(next), modify_to_eom_(modify_to_eom)
        {
        }

        explicit constexpr roll(std::chrono::day day) noexcept
            : roll(day, 0, false)
        {
        }

        constexpr roll(std::chrono::day day, std::int32_t next) noexcept
            : roll(day, next, false)
        {
        }

        constexpr roll(std::chrono::day day, bool modifiy_to_eom) noexcept
            : roll(day, 0, false)
        {
        }

        constexpr this_type& operator =(const this_type&) noexcept = default;
        constexpr this_type& operator =(this_type&&) noexcept = default;

    // -------------------------------------------------------------------------
    //  adjustment behavior
    //
        constexpr std::chrono::sys_days operator()(const std::chrono::sys_days& d) const
        {
            const std::chrono::year_month_day ymd {d};
            const auto ym = std::chrono::year_month {ymd.year(), ymd.month()}
                + std::chrono::months{ymd.day() <= day_ ? next_ : next_ + 1};
            const auto rolled = ym / day_;

            if (modify_to_eom_ && !rolled.ok()) {
                return std::chrono::year_month_day_last(rolled.year(), std::chrono::month_day_last{rolled.month()});
            }
            else {
                return std::chrono::sys_days(rolled);
            }
        }
        
    // -------------------------------------------------------------------------
    //  get
    //
        constexpr const std::chrono::day& day() const noexcept { return day_; }
        constexpr std::int32_t distance() const noexcept { return next_; }
        constexpr bool modify_to_eom() const noexcept { return modify_to_eom_; }

    private:
        std::chrono::day day_;
        std::int32_t next_ = 0;
        bool modify_to_eom_ = false;

    }; // class roll

} // namespace egret::chrono
