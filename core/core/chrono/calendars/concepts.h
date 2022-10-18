#pragma once

#include <chrono>
#include <ranges>
#include <set>
#include <string>
#include "core/concepts/range_of.h"

namespace egret_detail::cal_impl {
    void get_additional_holidays(auto&&, auto&&) = delete;

    template <typename Src>
    concept get_additional_holidays_mf = requires (const Src& src, std::string_view code) {
        { src.get_additional_holidays(code) } -> std::convertible_to<std::optional<std::vector<std::chrono::sys_days>>>;
    };

    template <typename Src>
    concept get_additional_holidays_adl = requires (const Src& src, std::string_view code) {
        { get_additional_holidays(src, code) } -> std::convertible_to<std::optional<std::vector<std::chrono::sys_days>>>;
    };

    class get_additional_holidays_t {
    public:
        template <typename Src>
            requires get_additional_holidays_mf<Src> || get_additional_holidays_adl<Src>
        constexpr std::optional<std::vector<std::chrono::sys_days>> operator()(const Src& src, std::string_view code) const
        {
            if constexpr (get_additional_holidays_mf<Src>) {
                return src.get_additional_holidays(code);
            }
            else {
                return get_additional_holidays(src, code);
            }
        }
    };

    void get_additional_businessdays(auto&&, auto&&) = delete;

    template <typename Src>
    concept get_additional_businessdays_mf = requires (const Src& src, std::string_view code) {
        { src.get_additional_businessdays(code) } -> std::convertible_to<std::optional<std::vector<std::chrono::sys_days>>>;
    };

    template <typename Src>
    concept get_additional_businessdays_adl = requires (const Src& src, std::string_view code) {
        { get_additional_businessdays(src, code) } -> std::convertible_to<std::optional<std::vector<std::chrono::sys_days>>>;
    };

    class get_additional_businessdays_t {
    public:
        template <typename Src>
            requires get_additional_businessdays_mf<Src> || get_additional_businessdays_adl<Src>
        constexpr std::optional<std::vector<std::chrono::sys_days>> operator()(const Src& src, std::string_view code) const
        {
            if constexpr (get_additional_businessdays_mf<Src>) {
                return src.get_additional_businessdays(code);
            }
            else {
                return get_additional_businessdays(src, code);
            }
        }
    };

} // namespace egret_detail::cal_impl

namespace egret::chrono::inline cpo {
// -----------------------------------------------------------------------------
//  [cpo] has_calendar_data
//  [cpo] get_additional_holidays
//  [cpo] get_additional_businessdays
// -----------------------------------------------------------------------------
    inline constexpr auto get_additional_holidays = egret_detail::cal_impl::get_additional_holidays_t {};
    inline constexpr auto get_additional_businessdays = egret_detail::cal_impl::get_additional_businessdays_t {};

} // namespace egret::chrono::inline cpo

namespace egret::cpt {
// -----------------------------------------------------------------------------
//  [concept] calendar_source
// -----------------------------------------------------------------------------
    template <typename Src>
    concept calendar_source = 
        requires (const Src& source, std::string_view code) {
            { chrono::get_additional_holidays(source, code) } 
                -> std::convertible_to<std::optional<std::vector<std::chrono::sys_days>>>;
            { chrono::get_additional_businessdays(source, code) } 
                -> std::convertible_to<std::optional<std::vector<std::chrono::sys_days>>>;
        };

} // namespace egret::cpt
