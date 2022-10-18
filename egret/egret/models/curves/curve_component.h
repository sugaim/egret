#pragma once

#include <string>
#include <nlohmann/json_fwd.hpp>
#include "core/utils/json_utils/j2obj.h"
#include "concepts.h"

namespace egret::model {
// -----------------------------------------------------------------------------
//  [struct] curve_component
// -----------------------------------------------------------------------------
    template <cpt::yield_curve C>
    struct curve_component {
        std::string name;
        double multiplier = 1;
        C curve;
    };

// -----------------------------------------------------------------------------
//  [struct] curve_component_dto
// -----------------------------------------------------------------------------
    struct curve_component_dto {
        std::string name;
        double multiplier = 1;
    };

} // namespace egret::model

namespace nlohmann {
    template <>
    struct adl_serializer<egret::model::curve_component_dto> {
        using target_type = egret::model::curve_component_dto;

        template <typename Json>
        static target_type from_json(const Json& j)
        {
            namespace j2obj = egret::util::j2obj;
            constexpr auto deser = j2obj::construct<target_type>(
                "name" >> j2obj::string,
                "multiplier" >> j2obj::number
            );
            return deser(j);
        }
        template <typename Json>
        static void to_json(Json& j, const target_type& obj)
        {
            j["name"] = obj.name;
            j["multipler"] = obj.multiplier;
        }
    };

} // namespace nlohmann
