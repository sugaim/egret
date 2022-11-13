#pragma once

#include <type_traits>
#include <concepts>
#include <expected>
#include <chrono>
#include <tuple>
#include <variant>
#include <spanstream>
#include "core/concepts/constructible_to.h"
#include "core/assertions/exception.h"
#include "core/utils/string_utils/trim.h"
#include "core/utils/string_utils/case_insensitive.h"
#include "string_value_map_of.h"

namespace egret_detail::parse_impl {
    struct outof_range_argument {};
    template <std::size_t i, typename ...Args>
    using type_at = std::conditional_t<
        i < sizeof...(Args),
        std::tuple_element<i, std::tuple<Args...>>,
        std::type_identity<outof_range_argument>
    >::type;

    void parse(auto&&, auto&&) = delete;

    enum class category {
        boolean,
        character,
        number,
        datetime,
        string_value_mappable,
        parse_mf,
        parse_adl,
        not_defined,
    };

    template <typename T, typename Char, typename ...Args>
    consteval category dispatch()
    {
        using enum category;
        constexpr std::size_t args_size = sizeof...(Args);
        if constexpr (std::same_as<T, bool> && args_size == 0) {
            return boolean;
        }
        if constexpr (std::same_as<T, Char> && args_size == 0) {
            return character;
        }
        if constexpr (std::is_arithmetic_v<T> && args_size == 0) {
            return number;
        }
        if constexpr (std::floating_point<T> && args_size == 1 &&
                      std::convertible_to<type_at<0, Args...>, std::chars_format>) {
            return number;
        }
        if constexpr (std::integral<T> && args_size == 1 &&
                      std::convertible_to<type_at<0, Args...>, int>) {
            return number;
        }
        if constexpr (args_size == 1 &&
                      requires (const type_at<0, Args...> fmt, T& obj) { std::chrono::parse(fmt, obj); }) {
            return datetime;
        }
        if constexpr (egret::cpt::basic_string_value_mappable<T, Char> &&
                      egret::util::enable_string_value_mappable_parser<T, Char>) {
            return string_value_mappable;
        }
        if constexpr (
            requires (std::basic_string_view<Char> sv, Args&& ...args) { 
                {T::parse(sv, std::forward<Args>(args)...)} -> egret::cpt::constructible_to<T>; 
            }
        ) 
        {
            return parse_mf;
        }
        if constexpr (
            requires (std::type_identity<T> tag, std::basic_string_view<Char> sv, Args&& ...args) {
                { parse(tag, sv, std::forward<Args>(args)...) } -> egret::cpt::constructible_to<T>; 
            }
        ) {
            return parse_adl;
        }
        return not_defined;
    }

    template <typename T, typename Char, category cat>
    struct parse_trait {};

    template <typename T, typename Char, typename ...Args>
    class binded_basic_parse_t {
    private:
        using this_type = binded_basic_parse_t;
        using tuple_t = std::tuple<Args...>;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns  
    //
        binded_basic_parse_t() = delete;
        constexpr binded_basic_parse_t(const this_type&) 
            requires std::is_copy_constructible_v<tuple_t> = default;
        constexpr binded_basic_parse_t(this_type&&)
            noexcept(std::is_nothrow_move_constructible_v<tuple_t>)
            requires std::is_move_constructible_v<tuple_t> = default;

        template <typename ...AArgs>
        constexpr binded_basic_parse_t(AArgs&& ...args)
            noexcept((std::is_nothrow_constructible_v<Args, AArgs> && ...))
            : args_(std::forward<AArgs>(args)...)
        {
        }

        constexpr this_type& operator =(const this_type&)
            requires std::is_copy_assignable_v<tuple_t> = default;
        constexpr this_type& operator =(this_type&&)
            noexcept(std::is_nothrow_move_constructible_v<tuple_t>)
            requires std::is_move_assignable_v<tuple_t> = default;

    // -------------------------------------------------------------------------
    //  operator()
    //
        template <std::convertible_to<std::basic_string_view<Char>> String>
            requires (parse_impl::dispatch<T, Char, const Args&...>() != category::not_defined)
        constexpr auto operator()(const String& string) const & noexcept
            -> std::expected<T, egret::exception>;

        template <std::convertible_to<std::basic_string_view<Char>> String>
            requires (parse_impl::dispatch<T, Char, Args...>() != category::not_defined)
        constexpr auto operator()(const String& string) && noexcept
            -> std::expected<T, egret::exception>;
            
    private:
        std::tuple<Args...> args_;

    }; // class binded_basic_parse_t

    template <typename T, typename Char>
        requires (std::same_as<Char, char> || std::same_as<Char, wchar_t>)
    class basic_parse_t {
    public:
        template <typename ...Args>
        constexpr auto bind(Args&& ...args) const
        {
            using result_t = binded_basic_parse_t<T, Char, std::unwrap_reference_t<std::decay_t<Args>>...>;
            return result_t(std::forward<Args>(args)...);
        }

        template <std::convertible_to<std::basic_string_view<Char>> String, typename ...Args>
            requires (parse_impl::dispatch<T, Char, Args...>() != category::not_defined)
        constexpr auto operator()(const String& string, Args&& ...args) const noexcept
            -> std::expected<T, egret::exception>
        {
            try {
                using parser = parse_trait<T, Char, parse_impl::dispatch<T, Char, Args...>()>;
                std::basic_string_view<Char> sv = string; 
                sv = egret::util::trim(sv);
                return std::expected<T, egret::exception>(parser::apply(sv, std::forward<Args>(args)...));
            }
            catch (const egret::exception& e) {
                return std::expected<T, egret::exception>(std::unexpect, e);
            }
            catch (const std::exception& e) {
                return std::expected<T, egret::exception>(
                    std::unexpect, 
                    egret::exception(e.what())
                );
            }
            catch (...) {
                return std::expected<T, egret::exception>(
                    std::unexpect, 
                    egret::exception("Parse error due to unexpected error.")
                );
            }
        }

    }; // class basic_parse_t

    template <typename T, typename Char, typename ...Args>
    template <std::convertible_to<std::basic_string_view<Char>> String>
        requires (parse_impl::dispatch<T, Char, const Args&...>() != category::not_defined)
    constexpr auto binded_basic_parse_t<T, Char, Args...>::operator()(const String& string) const & noexcept
        -> std::expected<T, egret::exception>
    {
        const auto f = [&string](const auto& ...args) {
            constexpr auto parser = basic_parse_t<T, Char> {};
            return parser(string, args...);
        };
        return std::apply(f, args_);
    }

    template <typename T, typename Char, typename ...Args>
    template <std::convertible_to<std::basic_string_view<Char>> String>
        requires (parse_impl::dispatch<T, Char, Args...>() != category::not_defined)
    constexpr auto binded_basic_parse_t<T, Char, Args...>::operator()(const String& string) && noexcept
        -> std::expected<T, egret::exception>
    {
        const auto f = [&string](auto&& ...args) {
            constexpr auto parser = basic_parse_t<T, Char> {};
            return parser(string, std::forward<decltype(args)>(args)...);
        };
        return std::apply(f, std::move(args_));
    }

    template <typename T, typename Char, typename ...Args>
    class binded_from_basic_string_t {
    private:
        using this_type = binded_from_basic_string_t;
        using tuple_t = std::tuple<Args...>;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns  
    //
        binded_from_basic_string_t() = delete;
        constexpr binded_from_basic_string_t(const this_type&) 
            requires std::is_copy_constructible_v<tuple_t> = default;
        constexpr binded_from_basic_string_t(this_type&&)
            noexcept(std::is_nothrow_move_constructible_v<tuple_t>)
            requires std::is_move_constructible_v<tuple_t> = default;

        template <typename ...AArgs>
        constexpr binded_from_basic_string_t(AArgs&& ...args)
            noexcept((std::is_nothrow_constructible_v<Args, AArgs> && ...))
            : args_(std::forward<AArgs>(args)...)
        {
        }

        constexpr this_type& operator =(const this_type&)
            requires std::is_copy_assignable_v<tuple_t> = default;
        constexpr this_type& operator =(this_type&&)
            noexcept(std::is_nothrow_move_constructible_v<tuple_t>)
            requires std::is_move_assignable_v<tuple_t> = default;

    // -------------------------------------------------------------------------
    //  operator()
    //
        template <std::convertible_to<std::basic_string_view<Char>> String>
            requires (parse_impl::dispatch<T, Char, const Args&...>() != category::not_defined)
        constexpr T operator()(const String& string) const &;

        template <std::convertible_to<std::basic_string_view<Char>> String>
            requires (parse_impl::dispatch<T, Char, Args...>() != category::not_defined)
        constexpr T operator()(const String& string) &&;
            
    private:
        std::tuple<Args...> args_;

    }; // class binded_from_basic_string_t
    
    template <typename T, typename Char>
    class from_basic_string_t {
    private:
        using this_type = from_basic_string_t;
        static constexpr auto parser = basic_parse_t<T, Char> {};

    public:
        template <typename ...Args>
        constexpr auto bind(Args&& ...args) const
        {
            using result_t = binded_from_basic_string_t<T, Char, std::unwrap_reference_t<std::decay_t<Args>>...>;
            return result_t(std::forward<Args>(args)...);
        }
        
        template <std::convertible_to<std::basic_string_view<Char>> String, typename ...Args>
            requires (parse_impl::dispatch<T, Char, Args...>() != category::not_defined)
        constexpr T operator()(const String& string, Args&& ...args) const
        {
            auto result = this_type::parser(string, std::forward<Args>(args)...);
            return result ? *std::move(result) : throw std::move(result).error().record_stacktrace();
        }

    }; // class from_basic_string_t
    
    template <typename T, typename Char, typename ...Args>
    template <std::convertible_to<std::basic_string_view<Char>> String>
        requires (parse_impl::dispatch<T, Char, const Args&...>() != category::not_defined)
    constexpr T binded_from_basic_string_t<T, Char, Args...>::operator()(const String& string) const &
    {
        const auto f = [&string](const auto& ...args) {
            constexpr auto parser = from_basic_string_t<T, Char> {};
            return parser(string, args...);
        };
        return std::apply(f, args_);
    }

    template <typename T, typename Char, typename ...Args>
    template <std::convertible_to<std::basic_string_view<Char>> String>
        requires (parse_impl::dispatch<T, Char, Args...>() != category::not_defined)
    constexpr T binded_from_basic_string_t<T, Char, Args...>::operator()(const String& string) &&
    {
        const auto f = [&string](auto&& ...args) {
            constexpr auto parser = from_basic_string_t<T, Char> {};
            return parser(string, std::forward<decltype(args)>(args)...);
        };
        return std::apply(f, std::move(args_));
    }
    
} // namespace egret_detail::parse_impl

namespace egret::util::inline cpo {
// -----------------------------------------------------------------------------
//  [cpo] basic_parse
//  [cpo] parse
//  [cpo] wparse
// -----------------------------------------------------------------------------
    template <typename T, typename Char>
    inline constexpr auto basic_parse = egret_detail::parse_impl::basic_parse_t<T, Char> {};

    template <typename T>
    inline constexpr auto parse = cpo::basic_parse<T, char>;
    
    template <typename T>    
    inline constexpr auto wparse = cpo::basic_parse<T, wchar_t>;

// -----------------------------------------------------------------------------
//  [cpo] from_basic_string
//  [cpo] from_string
//  [cpo] from_wstring
// -----------------------------------------------------------------------------
    template <typename T, typename Char>
    inline constexpr auto from_basic_string = egret_detail::parse_impl::from_basic_string_t<T, Char> {};

    template <typename T>
    inline constexpr auto from_string = cpo::from_basic_string<T, char>;
    
    template <typename T>    
    inline constexpr auto from_wstring = cpo::from_basic_string<T, wchar_t>;

} // namespace egret::util::inline cpo

namespace egret::cpt {
// -----------------------------------------------------------------------------
//  [concept] basic_parsable
//  [concept] parsable
//  [concept] wparsable
// -----------------------------------------------------------------------------
    template <typename T, typename Char>
    concept basic_parsable =
        (std::same_as<Char, char> || std::same_as<Char, wchar_t>) &&
        requires (std::basic_string_view<Char> sv) {
            { util::from_basic_string<T, Char>(sv) } -> std::convertible_to<T>;
        };

    template <typename T>
    concept parsable = basic_parsable<T, char>;

    template <typename T>
    concept wparsable = basic_parsable<T, wchar_t>;

} // namespace egret::cpt


/*
    implementations
*/
namespace egret_detail::parse_impl {
    template <typename T, typename Char>
    struct parse_trait<T, Char, category::boolean> {
        static constexpr bool apply(std::basic_string_view<Char> sv)
        {
            const Char true_chars[] = {'t', 'r', 'u', 'e'};
            const Char false_chars[] = {'f', 'a', 'l', 's', 'e'};

            using sv_t = egret::util::case_insensitive_basic_string_view<Char>;
            const sv_t cisv {sv.data(), sv.size()};

            if (sv_t{true_chars, 4} == cisv) {
                return true;
            }
            if (sv_t{false_chars, 5} == cisv) {
                return false;
            }
            else [[unlikely]] {
                if constexpr (std::same_as<Char, char>) {
                    throw egret::exception("Invalid string for bool types. [given=\"{}\"]", sv);
                }
                else {
                    throw egret::exception("Invalid string for bool types.");
                }
            }
        }
    };

    template <typename T, typename Char>
    struct parse_trait<T, Char, category::character> {
        static constexpr Char apply(std::basic_string_view<Char> sv)
        {
            if (sv.size() != 1) {
                if constexpr (std::same_as<Char, char>) {
                    throw egret::exception("Length of char-like string must be 1. [size={}, string=\"{}\"]", sv.size(), sv);
                }
                else {
                    throw egret::exception("Length of char-like string must be 1. [size={}]", sv.size());
                }
            }
            return sv.front();
        }
    };

    template <typename T, typename Char>
    struct parse_trait<T, Char, category::datetime> {
        template <typename Fmt>
        static T apply(std::basic_string_view<Char> sv, const Fmt& fmt)
        {
            std::basic_string<Char> str (sv);
            const auto span = std::span<Char>(str);
            auto istream = std::basic_ispanstream<Char>(span);

            T result {};
            istream >> std::chrono::parse(fmt, result);

            return result;
        }
    };

    template <std::floating_point T, typename Char>
    struct parse_trait<T, Char, category::number> {
        static constexpr T apply(std::basic_string_view<Char> sv, std::chars_format fmt = std::chars_format::general)
        {
            std::variant<std::string_view, std::string> string;
            if (fmt == std::chars_format::general && 
                (sv.contains(static_cast<Char>('e') || sv.contains(static_cast<Char>('E'))))) 
            {
                fmt = std::chars_format::scientific;
            }
            if constexpr (std::same_as<Char, char>) {
                string = sv;
            }
            else {
                std::string buf {};
                buf.reserve(sv.size());
                for (const auto& ch : sv) {
                    buf.push_back(static_cast<char>(ch));
                }
                string = std::move(buf);
            }
            const std::string_view parse_target = string.index() == 0 
                ? std::get<0>(string)                   // std::string_view
                : std::string_view(std::get<1>(string));// std::string

            T result {};
            using namespace std::string_view_literals;
            const auto [_, err] = std::from_chars(parse_target.data(), parse_target.data() + parse_target.size(), result, fmt);
            if (err != std::errc {}) [[unlikely]] {
                throw egret::exception(
                    "Fail to parse as string as floating point. [string=\"{}\", format={}]", parse_target, 
                        fmt == std::chars_format::fixed ? "fixed"sv :
                        fmt == std::chars_format::hex ? "hex"sv :
                        fmt == std::chars_format::scientific ? "scientific"sv :
                        "generic"sv
                );
            }
            return result;
        }
    };

    template <std::integral T, typename Char>
    struct parse_trait<T, Char, category::number> {
        static constexpr T apply(std::basic_string_view<Char> sv, const int base = 10)
        {
            std::variant<std::string_view, std::string> string;
            if constexpr (std::same_as<Char, char>) {
                string = sv;
            }
            else {
                std::string buf {};
                buf.reserve(sv.size());
                for (const auto& ch : sv) {
                    buf.push_back(static_cast<char>(ch));
                }
                string = std::move(buf);
            }
            const std::string_view parse_target = string.index() == 0 
                ? std::get<0>(string)                   // std::string_view
                : std::string_view(std::get<1>(string));// std::string

            T result {};
            const auto [_, err] = std::from_chars(parse_target.data(), parse_target.data() + parse_target.size(), result, base);
            if (err != std::errc {}) [[unlikely]] {
                throw egret::exception(
                    "Fail to parse as string as integer. [string=\"{}\", base={}]", parse_target, base
                );
            }
            return result;
        }
    };

    template <typename T, typename Char>
    struct parse_trait<T, Char, category::parse_adl> {
        template <typename ...Args>
        static constexpr T apply(std::basic_string_view<Char> sv, Args&& ...args)
        {
            return parse(std::type_identity<T> {}, sv, std::forward<Args>(args)...);
        }
    };
    
    template <typename T, typename Char>
    struct parse_trait<T, Char, category::parse_mf> {
        template <typename ...Args>
        static constexpr T apply(std::basic_string_view<Char> sv, Args&& ...args)
        {
            return T::parse(sv, std::forward<Args>(args)...);
        }
    };

    template <typename T, typename Char>
    struct parse_trait<T, Char, category::string_value_mappable> {
        static constexpr T apply(std::basic_string_view<Char> sv)
        {
            using sv_t = egret::util::case_insensitive_basic_string_view<Char>;
            const auto ci_sv = sv_t {sv.data(), sv.size()};
            const auto& map = egret::util::basic_string_value_map_of<Char>(std::type_identity<T>{});
            for (const auto& [str, value] : map) {
                const auto ci_str = sv_t{str.data(), str.size()};
                if (ci_str == ci_sv) {
                    return value;
                }
            }
            if constexpr (std::same_as<Char, char>) {
                throw egret::exception("Fail to find mapping from string_value_map for \"{}\"", sv);
            }
            else {
               throw egret::exception("Fail to find mapping from wstring_value_map for");
            }
        }
    };
    
} // namespace egret_detail::parse_impl
