#pragma once

#include <vector>
#include <chrono>
#include <nlohmann/json_fwd.hpp>

namespace egret::chrono::impl {
    std::vector<std::chrono::sys_days> get_days(
        const nlohmann::json& json,
        std::string_view property_name,
        std::string_view debug_tag
    );

} // namespace egret::chrono::impl
