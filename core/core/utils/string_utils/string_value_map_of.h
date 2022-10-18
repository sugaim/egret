#pragma once

#include <type_traits>
#include <concepts>
#include <tuple>
#include <ranges>
#include <vector>
#include <format>
#include <nlohmann/json_fwd.hpp>
#include "core/concepts/constructible_to.h"
#include "core/assertions/exception.h"
#include "core/utils/string_utils/case_insensitive.h"
#include "core/utils/string_utils/trim.h"

namespace egret_detail::svmap_impl {
    template <typename T, typename E, typename Char>
    concept string_value_map_item = 
        requires { std::tuple_size<std::remove_cvref_t<T>>::value; } &&
        std::tuple_size<std::remove_cvref_t<T>>::value == 2 &&
        requires (const T& item) {
            { get<0>(item) } -> egret::cpt::constructible_to<std::basic_string_view<Char>>;
            { get<1>(item) } -> egret::cpt::constructible_to<E>;
        };
    
    template <typename T, typename E, typename Char>
    concept string_value_map = 
        std::ranges::input_range<T> &&
        string_value_map_item<std::ranges::range_reference_t<T>, E, Char>;

    void string_value_map_of(auto) = delete;
    void wstring_value_map_of(auto) = delete;

    template <typename Char>
    class basic_string_value_map_of_t {};

    template <>
    class basic_string_value_map_of_t<char> {
    public:
        template <typename T>
            requires requires (std::type_identity<T> tag) {
                { string_value_map_of(tag) } -> string_value_map<T, char>;
            }
        const std::vector<std::pair<std::string, T>>& operator()(std::type_identity<T> tag) const
        {
            static const std::vector<std::pair<std::string, T>> data = [tag] {
                std::vector<std::pair<std::string, T>> result;
                const auto& map = string_value_map_of(tag);
                for (const auto& [key, value] : map) {
                    const std::string_view sv {key};
                    result.emplace_back(std::string(sv.data(), sv.size()), value);
                }
                return result;
            }();
            return data;
        }
    };
    
    template <>
    class basic_string_value_map_of_t<wchar_t> {
    public:
        template <typename T>
            requires requires (std::type_identity<T> tag) {
                { wstring_value_map_of(tag) } -> string_value_map<T, wchar_t>;
            }
        const std::vector<std::pair<std::wstring, T>>& operator()(std::type_identity<T> tag) const
        {
            static const std::vector<std::pair<std::wstring, T>> data = [tag] {
                std::vector<std::pair<std::wstring, T>> result;
                const auto& map = wstring_value_map_of(tag);
                for (const auto& [key, value] : map) {
                    const std::wstring_view sv {key};
                    result.emplace(std::string(sv.data(), sv.size()), value);
                }
                return result;
            };
            return data;
        }
    };
    
} // namespace egret_detail::svmap_impl

namespace egret::util::inline cpo {
// -----------------------------------------------------------------------------
//  [cpo] basic_string_value_map_of
// -----------------------------------------------------------------------------
    template <typename Char>
    inline constexpr auto basic_string_value_map_of = egret_detail::svmap_impl::basic_string_value_map_of_t<Char> {};

// -----------------------------------------------------------------------------
//  [cpo] string_value_map_of
//  [cpo] wstring_value_map_of
// -----------------------------------------------------------------------------
    inline constexpr auto string_value_map_of = basic_string_value_map_of<char>;
    inline constexpr auto wstring_value_map_of = basic_string_value_map_of<wchar_t>;

} // namespace egret::util::inline cpo

namespace egret::cpt {
// -----------------------------------------------------------------------------
//  [concept] basic_string_value_mappable
// -----------------------------------------------------------------------------
    template <typename T, typename Char>
    concept basic_string_value_mappable = 
        (std::same_as<Char, char> || std::same_as<Char, wchar_t>) &&
        requires (std::type_identity<T> tag) {
            { egret::util::basic_string_value_map_of<Char>(tag) }
                -> std::convertible_to<const std::vector<std::pair<std::basic_string<Char>, T>>&>;
        };

// -----------------------------------------------------------------------------
//  [concept] string_value_mappable
//  [concept] wstring_value_mappable
// -----------------------------------------------------------------------------
    template <typename T>
    concept string_value_mappable = basic_string_value_mappable<T, char>;

    template <typename T>
    concept wstring_value_mappable = basic_string_value_mappable<T, wchar_t>;

} // namespace egret::cpt

namespace egret::util {
// -----------------------------------------------------------------------------
//  [value] enable_string_value_mappable_formatter
// -----------------------------------------------------------------------------
    template <typename T, typename Char = char>
    inline constexpr bool enable_string_value_mappable_formatter
        = cpt::basic_string_value_mappable<T, Char>;

// -----------------------------------------------------------------------------
//  [value] enable_string_value_mappable_parser
// -----------------------------------------------------------------------------
    template <typename T, typename Char = char>
    inline constexpr bool enable_string_value_mappable_parser
        = cpt::basic_string_value_mappable<T, Char>;

// -----------------------------------------------------------------------------
//  [value] enable_string_value_mappable_json_serializer
// -----------------------------------------------------------------------------
    template <typename T>
    inline constexpr bool enable_string_value_mappable_json_serializer
        = cpt::string_value_mappable<T>;

} // namespace egret::util

namespace std {
    template <typename T, typename Char>
        requires 
            egret::cpt::basic_string_value_mappable<T, Char> &&
            std::equality_comparable<T> &&
            egret::util::enable_string_value_mappable_formatter<T, Char>
    struct formatter<T, Char> : std::formatter<std::basic_string<Char>, Char> {
        template <typename FmtCtxt>
        auto format(const T& obj, FmtCtxt& fc) const 
        {
            const auto& map = egret::util::basic_string_value_map_of<Char>(std::type_identity<T>());
            for (const auto& [str, value] : map) {
                if (value == obj) {
                    return std::formatter<std::basic_string<Char>, Char>::format(str, fc);
                }
            }
            throw std::format_error("string value map does not have mapping for given value.");
        }
    };

} // namespace std

namespace nlohmann {
    template <typename T>
        requires 
            egret::cpt::string_value_mappable<T> &&
            std::equality_comparable<T> &&
            egret::util::enable_string_value_mappable_json_serializer<T>
    struct adl_serializer<T> {

        template <typename Json>
        static T from_json(const Json& data)
        {
            static_assert(
                std::is_constructible_v<std::string_view, const typename Json::string_t&>,
                "json::string_t must"
            );
            if (!data.is_string()) {
                throw egret::exception("Json data is not a single string and can not be converted into a string value mappable object.")
                    .record_stacktrace();
            }

            using sv_t = egret::util::case_insensitive_basic_string_view<char>;
            const auto& json_str = data.template get_ref<const typename Json::string_t&>();
            std::string_view json_sv (json_str);
            json_sv = egret::util::trim(json_sv);

            const auto& map = egret::util::string_value_map_of(std::type_identity<T>());
            for (const auto& [str, value] : map) {
                const auto sv = egret::util::trim(str);
                if (sv_t(json_sv.data(), json_sv.size()) == sv_t(sv.data(), sv.size())) {
                    return value;
                }
            }
            throw egret::exception("string value map does not have mapping for given value.")
                .record_stacktrace();
        }
        
        template <typename Json>
        static void to_json(Json& data, const T& elem)
        {
            const auto& map = egret::util::string_value_map_of(std::type_identity<T>());
            for (const auto& [str, value] : map) {
                if (value == elem) {
                    data = static_cast<typename Json::string_t>(str);
                    return;
                }
            }
            throw egret::exception("string value map does not have mapping for given value.")
                .record_stacktrace();
        }
    };

} // namespace nlohmann
