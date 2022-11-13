#pragma once

#include <chrono>

namespace egret::chrono {
// -----------------------------------------------------------------------------
//  [class] adjustment_interface
// -----------------------------------------------------------------------------
    template <typename Derived>
    class adjustment_interface {
    public:
        template <typename TimePoint>
        friend TimePoint operator |(const TimePoint& d, const adjustment_interface& self)
            requires requires (const TimePoint& d, const Derived& self) {
                { self(d) } -> std::convertible_to<TimePoint>;
            }
        {
            return static_cast<const Derived&>(self)(d);
        }

    }; // class adjustment_interface

} // namespace egret::chrono
