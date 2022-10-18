#include <ranges>
#include <nlohmann/json.hpp>
#include "../holiday_adjustment.h"

namespace egret::mkt {
// -----------------------------------------------------------------------------
//  [class] holiday_adjustment
// -----------------------------------------------------------------------------
    std::chrono::sys_days holiday_adjustment::operator()(const chrono::calendar& cal, const std::chrono::sys_days& date) const
    {
        if (type_ == type::unadjust) [[unlikely]] {
            return date;
        }

        const auto is_holiday = [&cal](const std::chrono::sys_days& d) -> bool {
            const auto wd = std::chrono::weekday(d);
            return wd == std::chrono::Saturday || wd == std::chrono::Sunday
                ? !std::ranges::binary_search(cal.additional_businessdays(), d)
                :  std::ranges::binary_search(cal.additional_holidays(), d);
        };
        if (!is_holiday(date)) [[likely]] {
            return date;
        }

        const auto shifter = std::chrono::days(type_ == type::following || type_ == type::modified_following ? 1 : -1);

        auto result = date;
        do { result += shifter; } while (is_holiday(result));

        switch (type_) {
        case egret::mkt::holiday_adjustment::type::modified_following:
        case egret::mkt::holiday_adjustment::type::modified_preceeding:
            if (std::chrono::year_month_day(date).month() != std::chrono::year_month_day(result).month()) {
                do { result -= shifter; } while (is_holiday(result));
            }
            break;
        default:
            break;
        }
        return result;
    }

    std::chrono::sys_seconds holiday_adjustment::operator()(const chrono::calendar& cal, const std::chrono::sys_seconds& datetime) const
    {
        const auto date = std::chrono::time_point_cast<std::chrono::days>(datetime);
        const auto adjusted = (*this)(cal, date);
        return adjusted + (datetime - date);
    }

} // namespace egret::mkt