#pragma once

#include <chrono>

namespace egret::chrono {
// -----------------------------------------------------------------------------
//  [class] act360_t
//  [value] act360
// -----------------------------------------------------------------------------
    class act360_t {
    public:
        static constexpr const char* name = "act360";

        constexpr double operator()(const std::chrono::sys_days& from, const std::chrono::sys_days& to) const
        {
            constexpr double year_per_days = 1. / 360.;
            return (to - from).count() * year_per_days;
        }

        constexpr double operator()(const std::chrono::sys_seconds& from, const std::chrono::sys_seconds& to) const
        {
            constexpr double secs_per_days = 1. / (360 * 60 * 60 * 24);
            return (to - from).count() * secs_per_days;
        }

        constexpr auto operator<=>(const act360_t&) const noexcept = default;

    }; // class act360_t

    inline constexpr auto act360 = act360_t {};

} // namespace egret::chrono
