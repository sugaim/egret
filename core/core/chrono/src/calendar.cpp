#include <algorithm>
#include <vector>
#include <format>
#include <ranges>
#include <nlohmann/json.hpp>
#include "core/assertions/exception.h"
#include "core/assertions/assertion.h"
#include "core/utils/string_utils/parse.h"
#include "core/utils/string_utils/to_string.h"
#include "../calendars/calendar.h"
#include "calendar_json_impl.h"

namespace egret::chrono {
// -----------------------------------------------------------------------------
//  [fn] setup_additional_hols
// -----------------------------------------------------------------------------
    namespace {
        template <typename Pred>
        std::vector<std::chrono::sys_days> _setup_additional_xdays(
            std::vector<std::chrono::sys_days>&& data,
            const Pred& pred,
            bool for_additional_holidays
        )
        {
            std::ranges::sort(data);
            data.erase(std::unique(data.begin(), data.end()), data.end());

            if (const auto cnt = std::ranges::count_if(data, pred); cnt != 0) {
                std::string invalid_dates;
                invalid_dates.reserve(14 * cnt + 5);
                auto inserter = std::back_inserter(invalid_dates);
                for (const auto& invalid_date : data | std::views::filter(std::not_fn(pred))) {
                    inserter = std::format_to(inserter, "'{}', ", invalid_date);
                }
                throw exception(
                    "Invalid calendar data because its additional {} contains {}. [invalids=[{}]]",
                    for_additional_holidays ? "holidays" : "businessdays",
                    for_additional_holidays ? "weekdays" : "weekends",
                    invalid_dates
                ).record_stacktrace();
            }
            return std::vector<std::chrono::sys_days>(std::move(data));
        }

        inline constexpr auto _is_weekend = [](const std::chrono::sys_days& d) {
            const auto wd = std::chrono::weekday(d);
            return wd == std::chrono::Saturday || wd == std::chrono::Sunday;
        };
        inline constexpr auto _is_weekday = [](const std::chrono::sys_days& d) {
            return !_is_weekend(d);
        };

    } // namespace 

// -----------------------------------------------------------------------------
//  [struct] calendar::impl
// -----------------------------------------------------------------------------
    struct calendar::impl {
        calendar_identifier identifier;
        std::vector<std::chrono::sys_days> additional_hols;
        std::vector<std::chrono::sys_days> additional_bds;
    };

// -----------------------------------------------------------------------------
//  [class] calendar
// -----------------------------------------------------------------------------
    calendar::calendar() : calendar({}, {}, {}) {}

    calendar::calendar(
        calendar_identifier identifier,
        std::vector<std::chrono::sys_days> additional_hols,
        std::vector<std::chrono::sys_days> additional_bds
    )
        : impl_(std::make_shared<impl>(
            std::move(identifier),
            _setup_additional_xdays(std::move(additional_hols), _is_weekend, true),
            _setup_additional_xdays(std::move(additional_bds), _is_weekday, false)
        ))
    {
    }

    bool calendar::is_holiday(const std::chrono::sys_days& d) const
    {
        const auto wd = std::chrono::weekday(d);
        const auto is_weekday = wd != std::chrono::Saturday && wd != std::chrono::Sunday;
        if (is_weekday) [[likely]] {
            return std::ranges::binary_search(this->additional_holidays(), d);
        }
        else {
            return !std::ranges::binary_search(this->additional_businessdays(), d);
        }
    }
    bool calendar::is_businessday(const std::chrono::sys_days& d) const
    {
        return !this->is_holiday(d);
    }

    const calendar_identifier& calendar::identifier() const noexcept { return impl_->identifier; }
    const std::vector<std::chrono::sys_days>& calendar::additional_holidays() const noexcept { return impl_->additional_hols; }
    const std::vector<std::chrono::sys_days>& calendar::additional_businessdays() const noexcept { return impl_->additional_bds; }

// -----------------------------------------------------------------------------
//  [fn] count_businessdays
// -----------------------------------------------------------------------------
    std::size_t count_businessdays(const calendar& cal, const std::chrono::sys_days& from, const std::chrono::sys_days& to)
    {
        if (to < from) {
            throw exception(
                "Invalid arguments 'to' is before the 'from'. "
                "[from={}, to={}]", from, to
            ).record_stacktrace();
        }
        if (from == to) {
            return 0;
        }
        const std::size_t unadjusted_count = [&from, &to] {
            const auto from_wd = std::chrono::weekday(from);
            const auto to_wd = std::chrono::weekday(to);
            const auto fst_monday = from + (std::chrono::Monday - from_wd);
            const auto lstnxt_monday = to + (std::chrono::Monday - to_wd);

            const std::size_t adjust[7] = {0, 0, 4, 3, 2, 1, 0};
            return static_cast<std::size_t>((lstnxt_monday - fst_monday).count()) / 7 * 5 
                + adjust[from_wd.c_encoding()] 
                - adjust[to_wd.c_encoding()];
        }();

        const auto& sp_busdays = cal.additional_businessdays();
        const auto& sp_holdays = cal.additional_holidays();
        return unadjusted_count
            + std::ranges::distance(std::ranges::lower_bound(sp_busdays, from), std::ranges::lower_bound(sp_busdays, to))
            - std::ranges::distance(std::ranges::lower_bound(sp_holdays, from), std::ranges::lower_bound(sp_holdays, to));
    }
    
// -----------------------------------------------------------------------------
//  [fn] count_holidays
// -----------------------------------------------------------------------------
    std::size_t count_holidays(const calendar& cal, const std::chrono::sys_days& from, const std::chrono::sys_days& to)
    {
        if (to < from) {
            throw exception(
                "Invalid arguments 'to' is before the 'from'. "
                "[from={}, to={}]", from, to
            ).record_stacktrace();
        }
        if (from == to) {
            return 0;
        }
        return static_cast<std::size_t>((to - from).count()) - chrono::count_businessdays(cal, from, to);
    }
    
} // namespace egret::chrono
