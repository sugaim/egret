#include <nlohmann/json.hpp>
#include "core/assertions/assertion.h"
#include "core/utils/string_utils/parse.h"
#include "core/utils/json_utils/j2obj.h"
#include "calendar_json_impl.h"

namespace egret::chrono::impl {
    std::vector<std::chrono::sys_days> get_days(
        const nlohmann::json& json,
        std::string_view property_name,
        std::string_view debug_tag
    )
    {
        const auto deser = util::j2obj::array.to_vector_with(util::j2obj::string.parse_to<std::chrono::sys_days>("%F"));
        assertion(json.contains(property_name), "json data for {} does not have property \"{}\".", debug_tag, property_name);
        const auto& property = json.at(property_name);
        auto result = deser(property);
        return result;
    }
        
} // namespace egret::chrono::impl
