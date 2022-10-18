#pragma once

#include "egret/auto_link.h"
#include <type_traits>
#include <array>
#include <utility>
#include <chrono>
#include "core/utils/string_utils/string_value_map_of.h"
#include "core/chrono/calendars/calendar.h"

namespace egret::mkt {
// -----------------------------------------------------------------------------
//  [class] holiday_adjustment
// -----------------------------------------------------------------------------
    class holiday_adjuster;

    class holiday_adjustment {
    private:
        using this_type = holiday_adjustment;

        enum class type : int {
            unadjust,
            following,
            modified_following,
            preceeding,
            modified_preceeding,
        };

    public:
        static const this_type unadjust;
        static const this_type following;
        static const this_type modified_following;
        static const this_type preceeding;
        static const this_type modified_preceeding;

    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        constexpr holiday_adjustment() = default;
        constexpr holiday_adjustment(const this_type&) noexcept = default;
        constexpr holiday_adjustment(this_type&&) noexcept = default;

    private:
        constexpr holiday_adjustment(type type) noexcept : type_(type) {}

    public:
        constexpr this_type& operator =(const this_type&) noexcept = default;
        constexpr this_type& operator =(this_type&&) noexcept = default;

    // -------------------------------------------------------------------------
    //  comparison
    //
        constexpr auto operator<=>(const this_type&) const noexcept = default;

    // -------------------------------------------------------------------------
    //  operator()
    //
        std::chrono::sys_days operator()(const chrono::calendar& cal, const std::chrono::sys_days& date) const;
        std::chrono::sys_seconds operator()(const chrono::calendar& cal, const std::chrono::sys_seconds& datetime) const;

        holiday_adjuster operator()(const chrono::calendar& cal) const noexcept;

    // -------------------------------------------------------------------------
    //  cast
    //
        explicit constexpr operator int() const noexcept { return static_cast<int>(type_); }

    private:
        type type_ = type::unadjust;

    }; // class holiday_adjustment

    inline const holiday_adjustment holiday_adjustment::unadjust = holiday_adjustment::type::unadjust;
    inline const holiday_adjustment holiday_adjustment::following = holiday_adjustment::type::following;
    inline const holiday_adjustment holiday_adjustment::modified_following = holiday_adjustment::type::modified_following;
    inline const holiday_adjustment holiday_adjustment::preceeding = holiday_adjustment::type::preceeding;
    inline const holiday_adjustment holiday_adjustment::modified_preceeding = holiday_adjustment::type::modified_preceeding;

    constexpr auto string_value_map_of(std::type_identity<holiday_adjustment>)
    {
        using pair_t = std::pair<const char*, holiday_adjustment>;
        return std::array {
            pair_t {"unadjust", holiday_adjustment::unadjust},
            pair_t {"following", holiday_adjustment::following},
            pair_t {"modified_following", holiday_adjustment::modified_following},
            pair_t {"preceeding", holiday_adjustment::preceeding},
            pair_t {"modified_preceeding", holiday_adjustment::modified_preceeding},
        };
    }

// -----------------------------------------------------------------------------
//  [class] holiday_adjuster
// -----------------------------------------------------------------------------
    class holiday_adjuster {
    private:
        using this_type = holiday_adjuster;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns  
    //
        holiday_adjuster() = delete;
        holiday_adjuster(const this_type&) noexcept = default;
        holiday_adjuster(this_type&&) noexcept = default;

        holiday_adjuster(holiday_adjustment adj, chrono::calendar cal) noexcept: adj_(adj), cal_(cal) {}
        
        this_type& operator =(const this_type&) noexcept = default;
        this_type& operator =(this_type&&) noexcept = default;

    // -------------------------------------------------------------------------
    //  operator()
    //
        std::chrono::sys_days operator()(const std::chrono::sys_days& date) const { return adj_(cal_, date); }
        std::chrono::sys_seconds operator()(const std::chrono::sys_seconds& datetime) const { return adj_(cal_, datetime); }

    // -------------------------------------------------------------------------
    //  get
    //
        const holiday_adjustment& adjustment() const noexcept { return adj_; }
        const chrono::calendar& calendar() const noexcept { return cal_; }
        
    private:
        holiday_adjustment adj_;
        chrono::calendar cal_;

    }; // class holiday_adjuster

// -----------------------------------------------------------------------------
//  [class] holiday_adjustment
// -----------------------------------------------------------------------------
    inline holiday_adjuster holiday_adjustment::operator()(const chrono::calendar& cal) const noexcept { return { *this, cal }; }

} // namespace egret::mkt
