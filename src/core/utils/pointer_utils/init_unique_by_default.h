#pragma once

#include <memory>
#include <concepts>

namespace egret::util {
// -----------------------------------------------------------------------------
//  [class] init_unique_by_default_t
//  [value] init_unique_by_default
// -----------------------------------------------------------------------------
    class init_unique_by_default_t {
    public:
        template <typename T>
            requires std::default_initializable<T>
        constexpr operator std::unique_ptr<T>() const
        {
            return std::make_unique<T>();
        }

    }; // class init_unique_by_default_t

    inline constexpr auto init_unique_by_default = init_unique_by_default_t {};

} // namespace egret::util
