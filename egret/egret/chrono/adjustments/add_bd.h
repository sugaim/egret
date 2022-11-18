#pragma once

#include <chrono>
#include "core/chrono/calendars/calendar.h"
#include "adjustment_interface.h"

namespace egret::chrono {
// -----------------------------------------------------------------------------
//  [class] add_bd
// -----------------------------------------------------------------------------
    class add_bd : public adjustment_interface<add_bd> {
    private:
        using this_type = add_bd;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        add_bd() = delete;
        add_bd(const this_type&) noexcept = default;
        add_bd(this_type&&) noexcept = default;

        add_bd(std::int_fast32_t count, chrono::calendar cal) noexcept
            : count_(count), cal_(std::move(cal))
        {
        }
        add_bd(std::chrono::days count, chrono::calendar cal) noexcept
            : count_(count.count()), cal_(std::move(cal))
        {
        }

        this_type& operator =(const this_type&) noexcept = default;
        this_type& operator =(this_type&&) noexcept = default;

    // -------------------------------------------------------------------------
    //  operator()
    //
        auto operator()(const std::chrono::sys_days& d) const -> std::chrono::sys_days;        
        std::chrono::sys_seconds operator()(const std::chrono::sys_seconds& s) const
        {
            const auto d = std::chrono::time_point_cast<std::chrono::days>(s);
            return this->operator()(d) + (s - d);
        }

    // -------------------------------------------------------------------------
    //  get
    //
        std::int_fast32_t count() const noexcept { return count_; }
        const chrono::calendar& calendar() const noexcept { return cal_; }

    private:
        std::int_fast32_t count_;
        chrono::calendar cal_;

    }; // class add_bd

} // namespace egret::chrono