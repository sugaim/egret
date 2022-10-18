#pragma once

#include "core/auto_link.h"
#include <set>
#include <ranges>
#include <string>
#include <array>
#include <nlohmann/json_fwd.hpp>
#include "core/utils/json_utils/j2obj.h"
#include "core/utils/string_utils/string_value_map_of.h"

namespace egret::chrono {
// -----------------------------------------------------------------------------
//  [enum] calendar_combination
// -----------------------------------------------------------------------------
    enum class calendar_combination {
        all_open,
        any_open
    };

    constexpr auto string_value_map_of(std::type_identity<calendar_combination>)
    {
        using pair_t = std::pair<const char*, calendar_combination>;
        return std::array{
            pair_t {"all_open", calendar_combination::all_open},
            pair_t {"any_open", calendar_combination::any_open}
        };
    }

// -----------------------------------------------------------------------------
//  [struct] calendar_identifier
// -----------------------------------------------------------------------------
    struct calendar_identifier {
        std::set<std::string> codes;
        calendar_combination combination = calendar_combination::all_open;

        constexpr auto operator ==(const calendar_identifier& other) const
        {
            return codes.size() < 2 && codes.size() == other.codes.size()
                ? codes == other.codes
                : std::tie(codes, combination) == std::tie(other.codes, other.combination);
        }
        constexpr auto operator<=>(const calendar_identifier& other) const
        {
            return codes.size() < 2 && codes.size() == other.codes.size()
                ? codes <=> other.codes
                : std::tie(codes, combination) <=> std::tie(other.codes, other.combination);
        }
    };

} // namespace egret::chrono

namespace nlohmann {
// -----------------------------------------------------------------------------
//  [struct] adl_serializer
// -----------------------------------------------------------------------------
    template <>
    struct adl_serializer<egret::chrono::calendar_identifier> {
        template <typename Json>
        static egret::chrono::calendar_identifier from_json(const Json& j)
        {
            namespace j2obj = egret::util::j2obj;
            constexpr auto deserializer = j2obj::construct<egret::chrono::calendar_identifier>(
                "codes" >> j2obj::array.transform(std::views::transform(j2obj::string) | std::ranges::to<std::set>()),
                "combination_type" >> j2obj::get<egret::chrono::calendar_combination>
            );
            return deserializer(j);
        }
        template <typename Json>
        static void to_json(Json& j, const egret::chrono::calendar_identifier& key)
        {
            j["codes"] = key.codes;
            j["combination_type"] = key.combination;
        }
    };

} // namespace nlohmann