#pragma once

#include <type_traits>
#include <nlohmann/json_fwd.hpp>
#include "core/concepts/non_void.h"
#include "../concepts.h"

namespace egret::util::j2obj {
// -----------------------------------------------------------------------------
//  [class] json_deserializer_interface
// -----------------------------------------------------------------------------
    template <typename Derived>
    class json_deserializer_interface;

} // namespace egret::util::j2obj

namespace egret::cpt {
// -----------------------------------------------------------------------------
//  [concept] json_deserializer_interface
// -----------------------------------------------------------------------------
    template <typename T, typename Json>
    concept json_deserializer_interface = 
        cpt::basic_json<Json> &&
        requires (const T& deserializer, const Json& json) {
            { deserializer(json) } -> cpt::non_void;
        };

// -----------------------------------------------------------------------------
//  [concept] deserializable_json_with
// -----------------------------------------------------------------------------
    template <typename Json, typename T>
    concept deserializable_json_with = json_deserializer_interface<T, Json>;

} // namespace egret::cpt

namespace egret::util::j2obj {
// -----------------------------------------------------------------------------
//  [type] deserialize_result_t
//  [type] deserialize_result_value_t
// -----------------------------------------------------------------------------
    template <typename T, cpt::basic_json Json>
        requires cpt::json_deserializer_interface<T, Json>
    using deserialize_result_t = std::invoke_result_t<const T&, const Json&>;

    template <typename T, cpt::basic_json Json>
        requires cpt::json_deserializer_interface<T, Json>
    using deserialize_result_value_t = std::remove_cvref_t<deserialize_result_t<T, Json>>;

} // namespace egret::util::j2obj
