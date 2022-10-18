#pragma once

#include <chrono>

namespace egret::mkt {
// -----------------------------------------------------------------------------
//  [class] act365f_t
//  [value] act365f
// -----------------------------------------------------------------------------
    class act365f_t {
    public:
        static constexpr const char* name = "act365f";

        constexpr double operator()(const std::chrono::sys_days& from, const std::chrono::sys_days& to) const
        {
            constexpr double year_per_days = 1. / 365.;
            return (to - from).count() * year_per_days;
        }

        constexpr double operator()(const std::chrono::sys_seconds& from, const std::chrono::sys_seconds& to) const
        {
            constexpr double secs_per_days = 1. / (365 * 60 * 60 * 24);
            return (to - from).count() * secs_per_days;
        }

        constexpr auto operator<=>(const act365f_t&) const noexcept = default;

    }; // class act365f_t

    inline constexpr auto act365f = act365f_t {};

} // namespace egret::mkt
