#pragma once

#include <chrono>
#include <optional>
#include "core/assertions/exception.h"

namespace egret::chrono {
// -----------------------------------------------------------------------------
//  [class] stopwatch
// -----------------------------------------------------------------------------
    class stopwatch {
    private:
        using this_type = stopwatch;
        using clock = std::chrono::high_resolution_clock;
        using time_point = clock::time_point;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        constexpr stopwatch() = default;
        constexpr stopwatch(const this_type&) = default;
        constexpr stopwatch(this_type&&) = default;

        explicit constexpr stopwatch(time_point start) : start_(std::move(start)) {}

        constexpr this_type& operator =(const this_type&) = default;
        constexpr this_type& operator =(this_type&&) = default;

    // -------------------------------------------------------------------------
    //  start, stop, restart, reset
    //
        void start();
        void stop();
        void restart() noexcept;
        void reset() noexcept;

    // -------------------------------------------------------------------------
    //  is_started, is_recording, is_stopped
    //
        bool is_started() const noexcept { return start_.has_value(); }
        bool is_recording() const noexcept { return start_.has_value() && !end_.has_value(); }
        bool is_stopped() const noexcept { return start_.has_value() && end_.has_value(); }

    // -------------------------------------------------------------------------
    //  get  
    //
        const std::optional<time_point>& start_time() const noexcept { return start_; }
        const std::optional<time_point>& end_time() const noexcept { return end_; }

    // -------------------------------------------------------------------------
    //  elapsed
    //
        template <typename Duration>
        Duration elapsed() const
        {
            return start_ && end_ 
                ? std::chrono::duration_cast<Duration>(*end_ - *start_)
                : throw exception("elapsed time is not computable because stopwatch does not started or stopped yet.");
        }

        std::chrono::microseconds microseconds() const { return this->elapsed<std::chrono::microseconds>(); }
        std::chrono::milliseconds milliseconds() const { return this->elapsed<std::chrono::milliseconds>(); }
        std::chrono::seconds seconds() const { return this->elapsed<std::chrono::seconds>(); }
        std::chrono::minutes minutes() const { return this->elapsed<std::chrono::minutes>(); }
        std::chrono::hours hours() const { return this->elapsed<std::chrono::hours>(); }

    private:
        std::optional<time_point> start_;
        std::optional<time_point> end_;

    }; // class stopwatch

} // namespace egret::chrono
