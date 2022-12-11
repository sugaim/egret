#pragma once

#include "core/auto_link.h"
#include <map>
#include <vector>
#include <chrono>
#include <optional>
#include <ranges>
#include <nlohmann/json_fwd.hpp>
#include "core/utils/json_utils/concepts.h"
#include "core/utils/json_utils/j2obj.h"

namespace egret::chrono {
// -----------------------------------------------------------------------------
//  [class] json_map_calendar_source
// -----------------------------------------------------------------------------
    template <cpt::basic_json Json = nlohmann::json>
    class json_map_calendar_source {
    private:
        using this_type = json_map_calendar_source;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        json_map_calendar_source() = delete;
        json_map_calendar_source(const this_type&) = default;
        json_map_calendar_source(this_type&&) = default;

        explicit json_map_calendar_source(const Json& j)
            : json_map_()
        {
            for (const auto& [key, value] : j.template get_ref<const typename Json::object_t&>()) {
                json_map_.emplace(key, value);
            }
        }

        explicit json_map_calendar_source(const std::map<std::string, Json>& json_map)
            : json_map_()
        {
            for (const auto& [key, value] : json_map) {
                json_map_.emplace(key, value);
            }
        }
        explicit json_map_calendar_source(std::map<std::string, Json, std::less<>> json_map)
            : json_map_(std::move(json_map))
        {
        }

        this_type& operator =(const this_type&) = default;
        this_type& operator =(this_type&&) = default;

    // -------------------------------------------------------------------------
    //  calendar source behavior
    //
        std::optional<std::vector<std::chrono::sys_days>> get_additional_holidays(std::string_view code) const
        {
            if (!json_map_.contains(code)) {
                return std::nullopt;
            }
            constexpr auto array2dates = std::views::transform(util::j2obj::string.parse_to<std::chrono::sys_days>("%F"));
            constexpr auto deser =
                "additional_holidays" >> util::j2obj::array.transform(array2dates | std::ranges::to<std::vector>());
            return deser(json_map_.find(code)->second);
        }
        std::optional<std::vector<std::chrono::sys_days>> get_additional_businessdays(std::string_view code) const
        {
            if (!json_map_.contains(code)) {
                return std::nullopt;
            }
            constexpr auto array2dates = std::views::transform(util::j2obj::string.parse_to<std::chrono::sys_days>("%F"));
            constexpr auto deser =
                "additional_businessdays" >> util::j2obj::array.transform(array2dates | std::ranges::to<std::vector>());
            return deser(json_map_.find(code)->second);
        }

    // -------------------------------------------------------------------------
    //  get
    //
        const std::map<std::string, Json, std::less<>>& json_map() const noexcept { return json_map_; }

    private:
        std::map<std::string, Json, std::less<>> json_map_;

    }; // class json_map_calendar_source

} // namespace egret::chrono

namespace nlohmann {
// -----------------------------------------------------------------------------
//  adl_serializer<egret::chrono::json_map_calendar_source>
// -----------------------------------------------------------------------------
    template <typename Json>
    struct adl_serializer<egret::chrono::json_map_calendar_source<Json>> {
        static egret::chrono::json_map_calendar_source<Json> from_json(const Json& j)
        {
            return egret::chrono::json_map_calendar_source<Json>(j);
        }
        static void to_json(Json& j, const egret::chrono::json_map_calendar_source<Json>& self)
        {
            if constexpr (std::is_assignable_v<typename Json::object_t&, const std::map<std::string, Json, std::less<>>&>) {
                j = self.json_map();
            }
            else {
                typename Json::object_t obj {};
                for (const auto& [k, v] : self.json_map()) {
                    obj[k] = v;
                }
                j = std::move(obj);
            }
        }
    };

} // namespace nlohmann
