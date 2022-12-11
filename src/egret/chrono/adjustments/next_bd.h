#pragma once

#include <chrono>
#include "core/chrono/calendars/calendar.h"
#include "adjustment_interface.h"

namespace egret::chrono {
// -----------------------------------------------------------------------------
//  [class] next_bd
// -----------------------------------------------------------------------------
    class next_bd : public adjustment_interface<next_bd> {
    private:
        using this_type = next_bd;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        next_bd() = delete;
        next_bd(const this_type&) noexcept = default;
        next_bd(this_type&&) noexcept = default;

        explicit next_bd(chrono::calendar cal) noexcept
            : cal_(std::move(cal))
        {
        }

        this_type& operator =(const this_type&) noexcept = default;
        this_type& operator =(this_type&&) noexcept = default;

    // -------------------------------------------------------------------------
    //  operator()
    //
        auto operator()(const std::chrono::sys_days& d) const -> std::chrono::sys_days
        {
            auto result = d;
            while (cal_.is_holiday(d)) {
                result += std::chrono::days(1);
            }
            return result;
        }
        std::chrono::sys_seconds operator()(const std::chrono::sys_seconds& s) const
        {
            const auto d = std::chrono::time_point_cast<std::chrono::days>(s);
            return this->operator()(d) + (s - d);
        }

    // -------------------------------------------------------------------------
    //  get
    //
        const chrono::calendar& calendar() const noexcept { return cal_; }

    private:
        chrono::calendar cal_;

    }; // class next_bd

} // namespace egret::chrono
