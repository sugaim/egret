#pragma once

#include <concepts>
#include <chrono>

namespace egret_detail::qdt_impl {
    void quote_date(auto&&) = delete;

    class quote_date_t {
    public:
        template <typename T>
            requires requires (const T& obj) { { obj.quote_date() } -> std::convertible_to<std::chrono::sys_days>; }
        constexpr std::chrono::sys_days operator()(const T& obj) const
        {
            return obj.quote_date();
        }

        template <typename T>
            requires 
                (!requires (const T& obj) { { obj.quote_date() } -> std::convertible_to<std::chrono::sys_days>; }) &&
                requires (const T& obj) { { quote_date(obj) } -> std::convertible_to<std::chrono::sys_days>; }
        constexpr std::chrono::sys_days operator()(const T& obj) const
        {
            return quote_date(obj);
        }

    }; // class quote_date_t

    void quote_datetime(auto&&) = delete;

    class quote_datetime_t {
    public:
        template <typename T>
            requires requires (const T& obj) { { obj.quote_datetime() } -> std::convertible_to<std::chrono::sys_seconds>; }
        constexpr std::chrono::sys_seconds operator()(const T& obj) const
        {
            return obj.quote_datetime();
        }

        template <typename T>
            requires 
                (!requires (const T& obj) { { obj.quote_datetime() } -> std::convertible_to<std::chrono::sys_seconds>; }) &&
                requires (const T& obj) { { quote_datetime(obj) } -> std::convertible_to<std::chrono::sys_seconds>; }
        constexpr std::chrono::sys_seconds operator()(const T& obj) const
        {
            return quote_datetime(obj);
        }

    }; // class quote_datetime_t
    
} // namespace egret_detail::qdt_impl

namespace egret::model::inline cpo {
// -----------------------------------------------------------------------------
//  [cpo] quote_date
//  [cpo] quote_datetime
// -----------------------------------------------------------------------------
    inline constexpr auto quote_date = egret_detail::qdt_impl::quote_date_t {};
    inline constexpr auto quote_datetime = egret_detail::qdt_impl::quote_datetime_t {};

} // namespace egret::model::inline cpo
