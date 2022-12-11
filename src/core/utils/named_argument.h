#pragma once

#include <type_traits>
#include "core/concepts/non_reference.h"
#include "core/concepts/qualified.h"

namespace egret::util {
// -----------------------------------------------------------------------------
//  [class] value_wrapper
// -----------------------------------------------------------------------------
    template <cpt::non_reference T, typename Tag>
    class value_wrapper {
    public:
        using this_type = value_wrapper;

    private:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        value_wrapper() = delete;
        constexpr value_wrapper(const this_type&)
            noexcept(std::is_nothrow_copy_constructible_v<T>)
            requires std::is_copy_constructible_v<T> = default;
        constexpr value_wrapper(this_type&&) 
            noexcept(std::is_nothrow_move_constructible_v<T>)
            requires std::is_nothrow_move_constructible_v<T>= default;

        template <cpt::qualified<T> AT>
        explicit value_wrapper(AT&& value)
            : value_(std::forward<AT>(value))
        {
        }

        this_type& operator =(const this_type&)
            noexcept(std::is_nothrow_copy_assignable_v<T>)
            requires std::is_copy_assignable_v<T> = default;
        this_type& operator =(this_type&&)
            noexcept(std::is_nothrow_move_assignable_v<T>)
            requires std::is_move_assignable_v<T> = default;

    // -------------------------------------------------------------------------
    //  get
    //
        constexpr const T& get() const & noexcept { return value_; }
        constexpr T&& get() && noexcept { return std::move(value_); }

    public:
        T value_;

    }; // class value_wrapper

// -----------------------------------------------------------------------------
//  [class] named_argument
// -----------------------------------------------------------------------------
    template <typename ResultType>
    class named_argument {
    public:
        template <typename T>
            requires std::is_constructible_v<ResultType, T>
        constexpr ResultType operator =(T&& value) const
        {
            return ResultType(std::forward<T>(value));
        }

    }; // class named_argument

} // namespace egret::util
