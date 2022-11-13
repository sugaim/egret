#include <nlohmann/json.hpp>
#include "core/assertions/assertion.h"
#include "core/utils/string_utils/parse.h"
#include "calendar_json_impl.h"

namespace egret::chrono::impl {
    std::vector<std::chrono::sys_days> get_days(
        const nlohmann::json& json,
        std::string_view property_name,
        std::string_view debug_tag
    )
    {
        using array_t = nlohmann::json::array_t;

        assertion(json.contains(property_name), "json data for {} does not have property \"{}\".", debug_tag, property_name);
        const auto& property = json.at(property_name);
        assertion(property.is_array(), "json property \"{}\" for is not an array.", property_name, debug_tag);
        const auto& date_array = property;

        std::vector<std::chrono::sys_days> result;
        result.reserve(date_array.size());
        for (const auto& item : date_array) {
            assertion(item.is_string(), "some json array item of \"{}\" for {} is not a string.", property_name, debug_tag);
            const std::string& str = item;
            auto maybe_date = util::parse<std::chrono::sys_days>(str, "%F");
            assertion(maybe_date.has_value(), "some json array item of \"{}\" for {} is not in a date format.", property_name, debug_tag);
            result.push_back(*maybe_date);
        }
        return result;
    }
        
} // namespace egret::chrono::impl
