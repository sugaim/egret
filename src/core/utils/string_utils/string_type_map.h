#pragma once

#include <concepts>
#include <typeindex>
#include "core/concepts/constructible_to.h"
#include "core/assertions/fail.h"
#include "insensitive_strcmp.h"

namespace egret_detail::stmap_impl {
    template <typename T, typename Char>
    concept string_type_map_item = 
        requires { std::tuple_size<std::remove_cvref_t<T>>::value; } &&
        std::tuple_size<std::remove_cvref_t<T>>::value == 2 &&
        requires (const T& item) {
            { get<0>(item) } -> egret::cpt::constructible_to<std::basic_string_view<Char>>;
            { get<1>(item) } -> egret::cpt::constructible_to<std::type_index>;
        };
    
    template <typename T, typename Char>
    concept string_type_map = 
        std::ranges::range<T> &&
        string_type_map_item<std::ranges::range_reference_t<T>, Char>;

} // namespace egret_detail::stmap_impl

namespace egret::util {
// -----------------------------------------------------------------------------
//  [fn] string_to_type
// -----------------------------------------------------------------------------
    template <egret_detail::stmap_impl::string_type_map<char> Map>
    constexpr std::type_index string_to_type(const Map& map, std::string_view sv)
    {
        for (const auto& [name, type] : map) {
            std::string_view name_sv (name);
            if (util::insensitive_strcmp(name_sv, sv) == 0) {
                return type;
            }
        }
        egret::fail("Fail to find type from mapping between string and type. [given_string='{}']", sv);
        std::unreachable();
    }

    template <egret_detail::stmap_impl::string_type_map<wchar_t> Map>
    constexpr std::type_index string_to_type(const Map& map, std::wstring_view sv)
    {
        for (const auto& [name, type] : map) {
            std::wstring_view name_sv (name);
            if (util::insensitive_strcmp(name_sv, sv) == 0) {
                return type;
            }
        }
        egret::fail("Fail to find type from mapping between wstring and type.");
        std::unreachable();
    }
    
// -----------------------------------------------------------------------------
//  [fn] type_to_string
//  [fn] type_to_wstring
// -----------------------------------------------------------------------------
    template <egret_detail::stmap_impl::string_type_map<char> Map>
    constexpr std::string type_to_string(const Map& map, std::type_index t)
    {
        for (const auto& [name, type] : map) {
            if (type == t) {
                std::string_view name_sv (name);
                return std::string(name_sv.data(), name_sv.size());
            }
        }
        egret::fail("Fail to find string from mapping between string and type. [given_type='{}']", t.name());
        std::unreachable();
    }
    
    template <egret_detail::stmap_impl::string_type_map<char> Map>
    constexpr std::wstring type_to_wstring(const Map& map, std::type_index t)
    {
        for (const auto& [name, type] : map) {
            if (type == t) {
                std::wstring_view name_sv (name);
                return std::wstring(name_sv.data(), name_sv.size());
            }
        }
        egret::fail("Fail to find wstring from mapping between wstring and type. [given_type='{}']", t.name());
        std::unreachable();
    }
    
// -----------------------------------------------------------------------------
//  [fn] type_to_string_view
//  [fn] type_to_wstring_view
// -----------------------------------------------------------------------------
    template <egret_detail::stmap_impl::string_type_map<char> Map>
    constexpr std::string_view type_to_string_view(const Map& map, std::type_index t)
    {
        for (const auto& [name, type] : map) {
            if (type == t) {
                std::string_view name_sv (name);
                return name_sv;
            }
        }
        egret::fail("Fail to find string from mapping between string and type. [given_type='{}']", t.name());
    }
    
    template <egret_detail::stmap_impl::string_type_map<char> Map>
    constexpr std::wstring_view type_to_wstring_view(const Map& map, std::type_index t)
    {
        for (const auto& [name, type] : map) {
            if (type == t) {
                std::wstring_view name_sv (name);
                return name_sv;
            }
        }
        egret::fail("Fail to find wstring from mapping between wstring and type. [given_type='{}']", t.name());
    }

} // namespace egret::util
