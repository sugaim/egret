#include "../stopwatch.h"

namespace egret::chrono {
// -----------------------------------------------------------------------------
//  [class] stopwatch
// -----------------------------------------------------------------------------
    void stopwatch::start()
    {
        if (!start_.has_value()) {
            // not started
            start_ = clock::now();
            end_ = std::nullopt;
        }
    }

    void stopwatch::stop()
    {
        if (start_.has_value() && !end_.has_value()) {
            end_ = clock::now();
        }
    }

    void stopwatch::restart() noexcept
    {
        if (end_.has_value()) {
            end_ = std::nullopt;
        }
    }

    void stopwatch::reset() noexcept
    {
        start_ = std::nullopt;
        end_ = std::nullopt;
    }

} // namespace egret::chrono