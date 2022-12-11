#pragma once

#include <optional>
#include "core/config.h"
#include "core/concepts/non_reference.h"

namespace egret::util {
// -----------------------------------------------------------------------------
//  [class] debug_data
// -----------------------------------------------------------------------------
    template <cpt::non_reference T, bool is_debug = config::is_debug_mode>
        requires (!std::is_const_v<T>)
    class debug_data {
    private:
        using this_type = debug_data;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        constexpr debug_data() noexcept : data_(std::nullopt) {}
        constexpr debug_data(const this_type&) noexcept : debug_data() {}
        constexpr debug_data(this_type&&) noexcept : debug_data() {}

        constexpr this_type& operator =(const this_type&) const noexcept { return *this; }
        constexpr this_type& operator =(this_type&&) const noexcept { return *this; }

    // -------------------------------------------------------------------------
    //  emplace
    //
        template <typename ...Args>
            requires std::is_constructible_v<T, Args...>
        constexpr void emplace(Args&& ...args) const noexcept
        {
            try {
                data_.emplace(std::forward<Args>(args)...);
            }
            catch (...) {
                data_.reset();
            }
        }

        template <typename F, typename ...Args>
            requires std::is_invocable_r_v<T, F, Args...>
        constexpr void generate(F&& f, Args&& ...args) const noexcept
        {
            try {
                data_.emplace(std::invoke(std::forward<F>(f), std::forward<Args>(args)...));
            }
            catch (...) {
                data_.reset();
            }
        }
        
    private:
        mutable std::optional<T> data_;

    }; // class debug_data

    template <typename T>
    class debug_data<T, false> {
    private:
        using this_type = debug_data;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        constexpr debug_data() noexcept {}
        constexpr debug_data(const this_type&) noexcept : debug_data() {}
        constexpr debug_data(this_type&&) noexcept : debug_data() {}

        constexpr this_type& operator =(const this_type&) const noexcept { return *this; }
        constexpr this_type& operator =(this_type&&) const noexcept { return *this; }

    // -------------------------------------------------------------------------
    //  emplace
    //
        template <typename ...Args>
            requires std::is_constructible_v<T, Args...>
        constexpr void emplace(Args&& ...args) const noexcept
        {
        }
        
        template <typename F, typename ...Args>
            requires std::is_invocable_r_v<T, F, Args...>
        constexpr void generate(F&& f, Args&& ...args) const noexcept
        {
        }

    }; // class debug_data
    
} // namespace egret::util
