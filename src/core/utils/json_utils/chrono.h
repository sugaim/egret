#pragma once

#include <chrono>
#include <nlohmann/json_fwd.hpp>
#include "core/config.h"
#include "core/utils/string_utils/parse.h"
#include "core/utils/string_utils/to_string.h"
#include "j2obj.h"

namespace nlohmann {
    template <typename Clock>
        requires (!egret::config::disable_date_json_conversion)
    struct adl_serializer<std::chrono::time_point<Clock, std::chrono::days>> {
        using target_type = std::chrono::time_point<Clock, std::chrono::days>;

        template <typename Json>
        static void from_json(const Json& j, target_type& obj)
        {
            namespace j2obj = egret::util::j2obj;
            obj = j2obj::string.parse_to<target_type>("%F")(j);
        }
        template <typename Json>
        static void to_json(Json& j, const target_type& obj)
        {
            j = egret::util::to_string(obj);
        }
    };

} // namespace nlohmann