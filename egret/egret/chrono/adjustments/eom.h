#pragma once

#include <chrono>
#include <compare>
#include "adjustment_interface.h"

namespace egret::chrono {
// -----------------------------------------------------------------------------
//  [class] eom
// -----------------------------------------------------------------------------
    class eom : public adjustment_interface<eom> {
    private:
        using this_type = eom;        

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        constexpr eom() noexcept = default;
        constexpr eom(const this_type&) noexcept = default;
        constexpr eom(this_type&&) noexcept = default;

        explicit constexpr eom(std::int32_t next) noexcept
            : next_(next)
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
            std::chrono::year_month_day_last eom {ymd.year(), std::chrono::month_day_last{ymd.month()}};
            eom += std::chrono::months(next_);
            return eom;
        }
        constexpr std::chrono::sys_seconds operator()(const std::chrono::sys_seconds& s) const
        {
            const auto d = std::chrono::time_point_cast<std::chrono::days>(s);
            return this->operator()(d) + (s - d);
        }

    // -------------------------------------------------------------------------
    //  compare
    //
        constexpr auto operator<=>(const this_type&) const noexcept = default;

    // -------------------------------------------------------------------------
    //  get
    //
        constexpr std::int32_t distance() const noexcept { return next_; }

    private:
        std::int32_t next_ = 0;

    }; // class next_eom

} // namespace egret::chrono