#pragma once

namespace egret::config {
// -----------------------------------------------------------------------------
//  [value] is_debug_mode
// -----------------------------------------------------------------------------
#if defined(EGRET_DEBUG_MODE)
    inline constexpr bool is_debug_mode = true;
#elif defined(_MSC_VER) && defined(_DEBUG)
    inline constexpr bool is_debug_mode = true;
#else
    inline constexpr bool is_debug_mode = false;
#endif

// -----------------------------------------------------------------------------
//  [value] disable_date_json_conversion
// -----------------------------------------------------------------------------
    inline constexpr bool disable_date_json_conversion = false;

} // namespace egret::config
