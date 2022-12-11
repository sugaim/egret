#pragma once

#include <utility>
#include "core/concepts/non_reference.h"

namespace egret::util {
// -----------------------------------------------------------------------------
//  [class] construct_t
//  [value] construct
// -----------------------------------------------------------------------------
    template <typename T>
    class construct_t {
    public:
        template <typename ...Args>
            requires std::is_constructible_v<T, Args...>
        constexpr T operator()(Args&& ...args) const
        {
            return T(std::forward<Args>(args)...);
        }

    }; // class construct_t

    template <cpt::non_reference T>
    inline constexpr auto construct = construct_t<T> {};
    
// -----------------------------------------------------------------------------
//  [class] construct_from_tuple_t
//  [value] construct_from_tuple
// -----------------------------------------------------------------------------
    template <typename T>
    class construct_from_tuple_t {
    public:
        template <typename Tuple>
            requires requires (Tuple&& tpl) {
                std::make_from_tuple<T>(std::forward<Tuple>(tpl));
            }
        constexpr T operator()(Tuple&& tuple) const
        {
            return std::make_from_tuple<T>(std::forward<Tuple>(tuple));
        }

    }; // construct_from_tuple_t
    
    template <cpt::non_reference T>
    inline constexpr auto construct_from_tuple = construct_from_tuple_t<T> {};

} // namespace egret::util
