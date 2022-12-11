#pragma once

#include <utility>
#include <type_traits>
#include <compare>

namespace egret::util {
// -----------------------------------------------------------------------------
//  [class] member_data
// -----------------------------------------------------------------------------
    template <typename T>
    class member_data {
    private:
        using this_type = member_data;
        static_assert(std::is_same_v<T, std::remove_cvref_t<T>>, "This class expects T is a value type.");

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        constexpr member_data()
            noexcept(std::is_nothrow_default_constructible_v<T>)
            requires std::is_default_constructible_v<T> = default;
        constexpr member_data(const this_type&)
            noexcept(std::is_nothrow_copy_constructible_v<T>)
            requires std::is_copy_constructible_v<T> = default;
        constexpr member_data(this_type&&)
            noexcept(std::is_nothrow_move_constructible_v<T>)
            requires std::is_move_constructible_v<T> = default;

        constexpr explicit member_data(const T& obj) : obj_(obj) {}
        constexpr explicit member_data(T&& obj) : obj_(std::move(obj)) {}

        this_type& operator =(const this_type&)
            requires std::is_copy_assignable_v<T> = default;
        this_type& operator =(this_type&&)
            noexcept(std::is_nothrow_move_assignable_v<T>)
            requires std::is_move_assignable_v<T> = default;

    // -------------------------------------------------------------------------
    //  get
    //
        template <typename Self>
        constexpr auto&& get(this Self&& self) noexcept
        {
            return std::forward_like<Self>(self.obj_);
        }

    // -------------------------------------------------------------------------
    //  compare
    //
        constexpr auto operator<=>(const this_type&) const
            requires std::three_way_comparable<T> = default;

    private:
        T obj_;

    }; // class member_data

// -----------------------------------------------------------------------------
//  [class] member_data
// -----------------------------------------------------------------------------
    template <typename T>
    class member_data<T&> {
    private:
        using this_type = member_data;
        static_assert(std::is_same_v<T, std::remove_cvref_t<T>>, "This class expects T is a value type.");

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        constexpr member_data() = delete;
        constexpr member_data(const this_type&) noexcept = default;
        constexpr member_data(this_type&&) noexcept = default;

        constexpr member_data(T& ref) noexcept : obj_(ref) {}

        this_type& operator =(const this_type&) noexcept = default;
        this_type& operator =(this_type&&) noexcept = default;

    // -------------------------------------------------------------------------
    //  get
    //
        template <typename Self>
        constexpr auto&& get(this Self&& self) noexcept
        {
            // use forward_like after it is released!
            if constexpr (std::is_const_v<std::remove_reference_t<Self>>) {
                return std::as_const(obj_.get());
            }
            else {
                return obj_.get();
            }
        }

    // -------------------------------------------------------------------------
    //  compare
    //
        constexpr auto operator<=>(const this_type& other) const
            requires std::three_way_comparable<T>
        {
            return obj_.get() <=> other.obj_.get();
        }

    private:
        std::reference_wrapper<T> obj_;

    }; // class member_data
    
} // namespace egret::util