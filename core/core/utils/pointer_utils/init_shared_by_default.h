#pragma once

#include <memory>
#include <concepts>

namespace egret::util {
// -----------------------------------------------------------------------------
//  [class] init_shared_by_default_t
//  [value] init_shared_by_default
// -----------------------------------------------------------------------------
    class init_shared_by_default_t {
    public:
        template <typename T>
            requires std::default_initializable<T>
        constexpr operator std::shared_ptr<T>() const
        {
            return std::make_shared<T>();
        }

    }; // class init_shared_by_default_t

    inline constexpr auto init_shared_by_default = init_shared_by_default_t {};

} // namespace egret::util
