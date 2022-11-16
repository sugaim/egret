#include <fstream>
#include <nlohmann/json.hpp>
#include "core/assertions/assertion.h"
#include "core/utils/string_utils/parse.h"
#include "../calendars/json_directory_calendar_source.h"
#include "calendar_json_impl.h"

namespace egret::chrono {
    namespace {
        std::optional<std::vector<std::chrono::sys_days>> _get_days(
            const std::filesystem::path& dirpath,
            std::string_view code,
            std::string_view ext,
            std::string_view property_name
        )
        {
            std::ifstream ifs {(dirpath / std::format("{}{}", code, ext)).c_str()};
            if (!ifs.is_open()) {
                return std::nullopt;
            }
            const auto json = nlohmann::json::parse(ifs);
            return impl::get_days(json, property_name, code);
        }

    } // namespace 

// -----------------------------------------------------------------------------
//  [class] json_directory_calendar_source
// -----------------------------------------------------------------------------
    json_directory_calendar_source::json_directory_calendar_source(std::filesystem::path directory_path, std::string ext) noexcept
        : dirpath_(std::move(directory_path)), ext_(std::move(ext))
    {
    }

    json_directory_calendar_source::json_directory_calendar_source(std::filesystem::path directory_path)
        : json_directory_calendar_source(std::move(directory_path), ".json")
    {
    }

    auto json_directory_calendar_source::get_additional_holidays(std::string_view code) const
        -> std::optional<std::vector<std::chrono::sys_days>>
    {
        return _get_days(dirpath_, code, ext_, "additional_holidays");
    }
    auto json_directory_calendar_source::get_additional_businessdays(std::string_view code) const
        -> std::optional<std::vector<std::chrono::sys_days>>
    {
        return _get_days(dirpath_, code, ext_, "additional_businessdays");
    }

} // namespace egret::chrono
