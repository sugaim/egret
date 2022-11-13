#pragma once

#include <concepts>
#include <chrono>

namespace egret::cpt {
// -----------------------------------------------------------------------------
//  [concept] date_adjustment
// -----------------------------------------------------------------------------
    template <typename T, typename TimePoint = std::chrono::sys_days>
    concept date_adjustment = requires (const TimePoint & t, const T & adj) {
        { adj(t) } -> std::convertible_to<TimePoint>;
        { t | adj } -> std::convertible_to<TimePoint>;
    };

} // namespace egret::cpt
