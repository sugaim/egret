#pragma once

#include <tuple>
#include <variant>
#include <typeinfo>
#include <typeindex>
#include <nameof.hpp>

namespace egret::util {
// -----------------------------------------------------------------------------
//  [fn] variant_type_info
//  [fn] variant_type_index
// -----------------------------------------------------------------------------
    template <typename ...Ts>
    constexpr const std::type_info& variant_type_info(const std::variant<Ts...>& obj) noexcept
    {
        constexpr auto _typeid = [](const auto& obj) -> const std::type_info& {
            return typeid(obj);
        };
        return std::visit(_typeid, obj);
    }

    template <typename ...Ts>
    constexpr std::type_index variant_type_index(const std::variant<Ts...>& obj) noexcept
    {
        return std::type_index(util::variant_type_info(obj));
    }

// -----------------------------------------------------------------------------
//  [fn] nameof_variant_type
//  [fn] nameof_short_variant_type
// -----------------------------------------------------------------------------
    template <typename ...Ts>
    constexpr std::string_view nameof_variant_type(const std::variant<Ts...>& obj) noexcept
    {
        constexpr auto nameof = []<typename T>(const T&) -> std::string_view {
            return nameof::nameof_type<T>();
        };
        return std::visit(nameof, obj);
    }
    
    template <typename ...Ts>
    constexpr std::string_view nameof_short_variant_type(const std::variant<Ts...>& obj) noexcept
    {
        constexpr auto nameof = []<typename T>(const T&) -> std::string_view {
            return nameof::nameof_short_type<T>();
        };
        return std::visit(nameof, obj);
    }

} // namespace egret::util
