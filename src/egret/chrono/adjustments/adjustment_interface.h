#pragma once

#include <chrono>

namespace egret::chrono {
// -----------------------------------------------------------------------------
//  [class] adjustment_interface
// -----------------------------------------------------------------------------
    template <typename Derived>
    class adjustment_interface {
    public:


    }; // class adjustment_interface

    template <typename TimePoint, typename Derived>
    TimePoint operator |(const TimePoint& d, const adjustment_interface<Derived>& self)
        requires requires (const TimePoint& d, const Derived& self) {
            { self(d) } -> std::convertible_to<TimePoint>;
        }
    {
        return static_cast<const Derived&>(self)(d);
    }
    
} // namespace egret::chrono
