#pragma once

#include <utility>
#include <string_view>
#include "core/config.h"
#include "exception.h"

namespace egret {
// -----------------------------------------------------------------------------
//  [fn] assertion
// -----------------------------------------------------------------------------
    template <typename ...Args>
    void assertion(bool condition, std::string_view fmt, Args&& ...args)
    {
        if (!condition) [[unlikely]] {
            throw exception(fmt, std::forward<Args>(args)...).record_stacktrace();
        }
    }

    inline void assertion(bool condition)
    {
        if (!condition) [[unlikely]] {
            throw exception().record_stacktrace();
        }
    }

// -----------------------------------------------------------------------------
//  [fn] debug_assert
// -----------------------------------------------------------------------------
    template <typename ...Args>
    void debug_assert(bool condition, std::string_view fmt, Args&& ...args)
    {
        if constexpr (config::is_debug_mode) {
            egret::assertion(condition, fmt, std::forward<Args>(args)...);
        }
    }

    inline void debug_assert(bool condition)
    {
        if constexpr (config::is_debug_mode) {
            egret::assertion(condition);
        }
    }

} // namespace egret
