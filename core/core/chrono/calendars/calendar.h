#pragma once

#include "core/auto_link.h"
#include <vector>
#include <chrono>
#include <memory>
#include <nlohmann/json_fwd.hpp>
#include "core/assertions/exception.h"
#include "core/utils/json_utils/j2obj.h"
#include "core/utils/string_utils/to_string.h"
#include "calendar_identifier.h"

namespace egret::chrono {
// -----------------------------------------------------------------------------
//  [class] calendar
// -----------------------------------------------------------------------------
    class calendar {
    private:
        using this_type = calendar;
        struct impl;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        calendar() = delete;
        calendar(const this_type&) noexcept = default;
        calendar(this_type&&) noexcept = default;

        calendar(
            calendar_identifier identifier, 
            std::vector<std::chrono::sys_days> additional_holidays,      // weekdays, but holidays
            std::vector<std::chrono::sys_days> additional_businessdays   // weekends, but businessdays
        );

        this_type& operator =(const this_type&) noexcept = default;
        this_type& operator =(this_type&&) noexcept = default;

    // -------------------------------------------------------------------------
    //  get
    //
        const calendar_identifier& identifier() const noexcept;
        const std::vector<std::chrono::sys_days>& additional_holidays() const noexcept;
        const std::vector<std::chrono::sys_days>& additional_businessdays() const noexcept;

    private:
        std::shared_ptr<const impl> impl_;

    }; // class calendar

// -----------------------------------------------------------------------------
//  [fn] count_businessdays
// -----------------------------------------------------------------------------
    std::size_t count_businessdays(
        const calendar& cal, 
        const std::chrono::sys_days& from,
        const std::chrono::sys_days& to
    );

// -----------------------------------------------------------------------------
//  [fn] count_holidays
// -----------------------------------------------------------------------------
    std::size_t count_holidays(
        const calendar& cal,
        const std::chrono::sys_days& from,
        const std::chrono::sys_days& to
    );
    
} // namespace egret::chrono

namespace nlohmann {
// -----------------------------------------------------------------------------
//  adl_serializer<egret::chrono::calendar>
// -----------------------------------------------------------------------------
    template <>
    struct adl_serializer<egret::chrono::calendar> {
        template <typename Json>
        static egret::chrono::calendar from_json(const Json& j)
        {
            namespace j2obj = egret::util::j2obj;
            constexpr auto array2days = std::views::transform(j2obj::string.parse_to<std::chrono::sys_days>("%F")) | std::ranges::to<std::vector>();
            constexpr auto deserializer = j2obj::construct<egret::chrono::calendar>(
                "identifier" >> j2obj::get<egret::chrono::calendar_identifier>,
                "additional_holidays" >> j2obj::array.transform(array2days),
                "additional_businessdays" >> j2obj::array.transform(array2days)
            );
            return deserializer(j);
        }
        template <typename Json>
        static void to_json(Json& j, const egret::chrono::calendar& self)
        {
            constexpr auto d2j = [](const auto& d) { return Json(egret::util::to_string(d)); };
            j["identifier"] = self.identifier();
            j["additional_holidays"] = self.additional_holidays() | std::views::transform(d2j) | std::ranges::to<std::vector>();
            j["additional_businessdays"] = self.additional_businessdays() | std::views::transform(d2j) | std::ranges::to<std::vector>();
        }
    };

} // namespace nlohmann
