#pragma once

#include <array>
#include <type_traits>
#include <variant>
#include "core/utils/string_utils/string_value_map_of.h"
#include "daycounter_variant.h"
#include "act365f.h"
#include "act360.h"

namespace egret::mkt {
// -----------------------------------------------------------------------------
//  [type] daycounter
// -----------------------------------------------------------------------------
    using daycounter = daycounter_variant<std::tuple<std::chrono::sys_days, std::chrono::sys_seconds>,
        act365f_t,
        act360_t
    >;

    constexpr auto string_value_map_of(std::type_identity<daycounter>)
    {
        using pair_t = std::pair<const char*, daycounter>;
        return std::array {
            pair_t {act365f_t::name, act365f},
            pair_t {act360_t::name, act360}, 
        };
    }

} // namespace egret::mkt
