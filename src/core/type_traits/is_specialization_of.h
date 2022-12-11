#pragma once

#include <type_traits>

namespace egret {
// -----------------------------------------------------------------------------
//  [trait] is_specialization_of
//  [value] is_specialization_of_v
// -----------------------------------------------------------------------------
    template <template <typename...> typename TT, typename T>
    struct is_specialization_of : std::false_type {};

    template <template <typename...> typename TT, typename ...Args>
    struct is_specialization_of<TT, TT<Args...>> : std::true_type {};

    template <template <typename...> typename TT, typename T>
    struct is_specialization_of<TT, const T> : is_specialization_of<TT, T> {};

    template <template <typename...> typename TT, typename T>
    struct is_specialization_of<TT, volatile T> : is_specialization_of<TT, T> {};

    template <template <typename...> typename TT, typename T>
    struct is_specialization_of<TT, const volatile T> : is_specialization_of<TT, T> {};

    template <template <typename...> typename TT, typename T>
    inline constexpr bool is_specialization_of_v = is_specialization_of<TT, T>::value;

} // namespace egret
