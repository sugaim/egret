#pragma once

#include <type_traits>

namespace egret {
// -----------------------------------------------------------------------------
//  [struct] constructor_properties
// -----------------------------------------------------------------------------
    template <typename ...Ts>
    struct constructor_properties {
        static constexpr bool are_default_constructible_v
            = std::conjunction_v<std::is_default_constructible<Ts>...>;
        static constexpr bool are_nothrow_default_constructible_v
            = std::conjunction_v<std::is_nothrow_default_constructible<Ts>...>;

        static constexpr bool are_copy_constructible_v
            = std::conjunction_v<std::is_copy_constructible<Ts>...>;
        static constexpr bool are_nothrow_copy_constructible_v
            = std::conjunction_v<std::is_nothrow_copy_constructible<Ts>...>;

        static constexpr bool are_move_constructible_v
            = std::conjunction_v<std::is_move_constructible<Ts>...>;
        static constexpr bool are_nothrow_move_constructible_v
            = std::conjunction_v<std::is_nothrow_move_constructible<Ts>...>;

        template <typename ...Args>
        static constexpr bool is_each_constructible_from_v
            = std::conjunction_v<std::is_constructible<Ts, Args>...>;

        template <typename ...Args>
        static constexpr bool is_each_nothrow_constructible_from_v
            = std::conjunction_v<std::is_nothrow_constructible<Ts, Args>...>;
    };
    
// -----------------------------------------------------------------------------
//  [struct] assignment_properties
// -----------------------------------------------------------------------------
    template <typename ...Ts>
    struct assignment_properties {
        static constexpr bool are_copy_assignable_v
            = std::conjunction_v<std::is_copy_assignable<Ts>...>;
        static constexpr bool are_nothrow_copy_assignable_v
            = std::conjunction_v<std::is_nothrow_copy_assignable<Ts>...>;

        static constexpr bool are_move_assignable_v
            = std::conjunction_v<std::is_move_assignable<Ts>...>;
        static constexpr bool are_nothrow_move_assignable_v
            = std::conjunction_v<std::is_nothrow_move_assignable<Ts>...>;
    };

// -----------------------------------------------------------------------------
//  [struct] special_function_properties
// -----------------------------------------------------------------------------
    template <typename ...Ts>
    struct special_function_properties :
        constructor_properties<Ts...>,
        assignment_properties<Ts...> {};

} // namespace egret
