#include "../adjustments/holiday_adjustment.h"

namespace egret_detail::hadj_impl {
    struct is_busday_t {
        
        is_busday_t(const egret::chrono::calendar& cal) noexcept
            : additional_hds(&cal.additional_holidays()), additional_bds(&cal.additional_businessdays())
        {
        }

        const std::vector<std::chrono::sys_days>* additional_hds;
        const std::vector<std::chrono::sys_days>* additional_bds;

        bool operator()(const std::chrono::sys_days& d) const
        {
            const auto wd = std::chrono::weekday(d);
            return wd != std::chrono::Saturday && wd != std::chrono::Sunday
                ? !std::ranges::binary_search(*additional_hds, d)
                :  std::ranges::binary_search(*additional_bds, d);
        }
    };

// -----------------------------------------------------------------------------
//  [fn] adjust
// -----------------------------------------------------------------------------
    std::chrono::sys_days adjust(
        const std::chrono::sys_days& d,
        const egret::chrono::calendar& cal,
        type_constant<egret::chrono::holiday_adjustment_type::following>
    )
    {
        const auto is_busday = is_busday_t(cal);
        auto result = d;
        while (!is_busday(d)) {
            result += std::chrono::days(1);
        }
        return result;
    }

    std::chrono::sys_days adjust(
        const std::chrono::sys_days& d,
        const egret::chrono::calendar& cal,
        type_constant<egret::chrono::holiday_adjustment_type::preceeding>
    )
    {
        const auto is_busday = is_busday_t(cal);
        auto result = d;
        while (!is_busday(d)) {
            result -= std::chrono::days(1);
        }
        return result;
    }
    
    std::chrono::sys_days adjust(
        const std::chrono::sys_days& d,
        const egret::chrono::calendar& cal,
        type_constant<egret::chrono::holiday_adjustment_type::modified_following>
    )
    {
        auto result = adjust(d, cal, type_constant<egret::chrono::holiday_adjustment_type::following> {});
        if (std::chrono::year_month_day(d).month() != std::chrono::year_month_day(result).month()) [[unlikely]] {
            return adjust(
                result -= std::chrono::days(1), cal,
                type_constant<egret::chrono::holiday_adjustment_type::preceeding> {}
            );
        }
        else [[likely]] {
            return result;
        }
    }

    std::chrono::sys_days adjust(
        const std::chrono::sys_days& d,
        const egret::chrono::calendar& cal,
        type_constant<egret::chrono::holiday_adjustment_type::modified_preceeding>
    )
    {
        auto result = adjust(d, cal, type_constant<egret::chrono::holiday_adjustment_type::preceeding> {});
        if (std::chrono::year_month_day(d).month() != std::chrono::year_month_day(result).month()) [[unlikely]] {
            return adjust(
                result += std::chrono::days(1), cal,
                type_constant<egret::chrono::holiday_adjustment_type::following> {}
            );
        }
        else [[likely]] {
            return result;
        }
    }

    std::chrono::sys_days adjust(
        const std::chrono::sys_days& d,
        const egret::chrono::calendar& cal,
        egret::chrono::holiday_adjustment_type type
    )
    {
        switch (type) {
        case egret::chrono::holiday_adjustment_type::unadjust:
            return adjust(d, cal, type_constant<egret::chrono::holiday_adjustment_type::unadjust> {});

        case egret::chrono::holiday_adjustment_type::following:
            return adjust(d, cal, type_constant<egret::chrono::holiday_adjustment_type::following> {});

        case egret::chrono::holiday_adjustment_type::preceeding:
            return adjust(d, cal, type_constant<egret::chrono::holiday_adjustment_type::preceeding> {});

        case egret::chrono::holiday_adjustment_type::modified_following:
            return adjust(d, cal, type_constant<egret::chrono::holiday_adjustment_type::modified_following> {});

        case egret::chrono::holiday_adjustment_type::modified_preceeding:
            return adjust(d, cal, type_constant<egret::chrono::holiday_adjustment_type::modified_preceeding> {});
        }
        std::unreachable();
    }
    
    std::chrono::sys_seconds adjust(
        const std::chrono::sys_seconds& dt,
        const egret::chrono::calendar& cal,
        egret::chrono::holiday_adjustment_type type
    )
    {
        const auto d = std::chrono::time_point_cast<std::chrono::days>(dt);
        return hadj_impl::adjust(d, cal, type) + (dt - d);
    }
    
} // namespace egret_detail::hadj_impl

namespace egret::chrono {
// -----------------------------------------------------------------------------
//  [class] adjust
// -----------------------------------------------------------------------------
    auto adjust(const std::chrono::sys_days& d, const calendar& cal, holiday_adjustment_type type) -> std::chrono::sys_days
    {
        return egret_detail::hadj_impl::adjust(d, cal, type);
    }
    auto adjust(const std::chrono::sys_seconds& dt, const calendar& cal, holiday_adjustment_type type) -> std::chrono::sys_seconds
    {
        return egret_detail::hadj_impl::adjust(dt, cal, type);
    }
} // namespace egret::chrono