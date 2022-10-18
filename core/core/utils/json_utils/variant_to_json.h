#pragma once

#include <variant>
#include "concepts.h"

namespace egret::util {
// -----------------------------------------------------------------------------
//  [fn] variant_to_json
// -----------------------------------------------------------------------------
    template <cpt::basic_json Json, typename ...T>
        requires std::conjunction_v<std::is_assignable<Json&, const T&>...>
    void variant_to_json(Json& j, const std::variant<T...>& variant)
    {
        const auto to_json = [&j](const auto& obj) {
            j = obj;
        };
        std::visit(to_json, variant);
    }

} // namespace egret::util
