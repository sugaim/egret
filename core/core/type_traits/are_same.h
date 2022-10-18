#pragma once

#include <type_traits>

namespace egret {
// -----------------------------------------------------------------------------
//  [trait] are_same
//  [value] are_same_v
// -----------------------------------------------------------------------------
    template <typename ...T>
    struct are_same : std::true_type {};

    template <typename T, typename ...U>
    struct are_same<T, U...> : std::conjunction<std::is_same<T, U>...> {};

    template <typename ...T>
    inline constexpr bool are_same_v = are_same<T...>::value;

} // namespace egret
