#pragma once

#include <format>
#include <string>
#include <iterator>

namespace egret_detail::tostr_impl {
    template <typename T, typename Char, typename OIt = std::back_insert_iterator<std::basic_string<Char>>>
    concept formattable = 
        requires (
            T&& obj, 
            std::formatter<std::remove_cvref_t<T>, Char> fmt, 
            std::basic_format_context<OIt, Char> fmtctx,
            std::basic_format_parse_context<Char> prsctx
        ) {
            { fmt.format(std::forward<T>(obj), fmtctx) } -> std::output_iterator<Char>;
            { fmt.parse(prsctx) } -> std::same_as<typename std::basic_format_parse_context<Char>::iterator>;
        };

    template <typename Char>
    struct to_basic_string_t {
        template <formattable<Char> T>
        auto operator()(T&& obj) const -> std::basic_string<Char>
        {
            if constexpr (std::is_same_v<Char, char>) {
                return std::format("{}", std::forward<T>(obj));
            }
            else {
                return std::format(L"{}", std::forward<T>(obj));
            }
        }

        template <formattable<Char> T>
        auto operator()(T&& obj, std::basic_string_view<Char> fmt) const -> std::basic_string<Char>
        {
            if constexpr (std::is_same_v<Char, char>) {
                return std::vformat(std::format("{{:{}}}", fmt), std::forward<T>(obj));
            }
            else {
                return std::vformat(std::format(L"{{:{}}}", fmt), std::forward<T>(obj));
            }
        }
    };

} // namespace egret_detail::tostr_impl

namespace egret::util::inline cpo {
// -----------------------------------------------------------------------------
//  [cpo] to_basic_string
//  [cpo] to_string
//  [cpo] to_wstring
// -----------------------------------------------------------------------------
    template <typename Char>
        requires std::same_as<Char, char> || std::same_as<Char, wchar_t>
    inline constexpr auto to_basic_string = egret_detail::tostr_impl::to_basic_string_t<Char> {};

    inline constexpr auto to_string = to_basic_string<char>;
    inline constexpr auto to_wstring = to_basic_string<wchar_t>;

} // namespace egret::util::inline cpo

namespace egret::cpt {
// -----------------------------------------------------------------------------
//  [concept] to_stringable
// -----------------------------------------------------------------------------
    template <typename T, typename Char>
    concept to_stringable = requires (T&& obj) {
        { util::to_basic_string<Char>(std::forward<T>(obj)) } -> std::convertible_to<std::basic_string<Char>>;
    };

} // namespace egret::cpt
