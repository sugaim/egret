#include <fstream>
#include <mutex>
#include <nlohmann/json.hpp>
#include "core/assertions/assertion.h"
#include "core/utils/string_utils/parse.h"
#include "../calendars/json_directory_calendar_source.h"
#include "calendar_json_impl.h"

namespace egret::chrono {
    namespace {
        struct _json_file_info {
            std::filesystem::path path;
            std::filesystem::file_time_type timestamp;
            nlohmann::json data;
        };

        std::optional<std::vector<std::chrono::sys_days>> _get_days(
            const std::filesystem::path& dirpath,
            std::string_view code,
            std::string_view ext,
            std::string_view property_name,
            std::optional<_json_file_info>& cache_json
        )
        {
            auto path = dirpath / std::format("{}{}", code, ext);
            auto timestamp = std::filesystem::last_write_time(path);
            if (cache_json.has_value() && path == cache_json->path && timestamp == cache_json->timestamp) {
                return impl::get_days(cache_json->data, property_name, code);
            }
            std::ifstream ifs {path.c_str()};
            if (!ifs.is_open()) {
                return std::nullopt;
            }
            cache_json.emplace(std::move(path), std::move(timestamp), nlohmann::json::parse(ifs));
            return impl::get_days(cache_json->data, property_name, code);
        }

    } // namespace 

// -----------------------------------------------------------------------------
//  [class] json_directory_calendar_source
// -----------------------------------------------------------------------------
    struct json_directory_calendar_source::cache {
        std::mutex mutex;
        std::optional<_json_file_info> json;
    };

    json_directory_calendar_source::json_directory_calendar_source(std::filesystem::path directory_path, std::string ext) noexcept
        : dirpath_(std::move(directory_path)), ext_(std::move(ext)), cache_(std::make_shared<cache>())
    {
    }

    json_directory_calendar_source::json_directory_calendar_source(std::filesystem::path directory_path)
        : json_directory_calendar_source(std::move(directory_path), ".json")
    {
    }

    auto json_directory_calendar_source::get_additional_holidays(std::string_view code) const
        -> std::optional<std::vector<std::chrono::sys_days>>
    {
        std::lock_guard<std::mutex> _ {cache_->mutex};
        return _get_days(dirpath_, code, ext_, "additional_holidays", cache_->json);
    }
    auto json_directory_calendar_source::get_additional_businessdays(std::string_view code) const
        -> std::optional<std::vector<std::chrono::sys_days>>
    {
        std::lock_guard<std::mutex> _ {cache_->mutex};
        return _get_days(dirpath_, code, ext_, "additional_businessdays", cache_->json);
    }

} // namespace egret::chrono
