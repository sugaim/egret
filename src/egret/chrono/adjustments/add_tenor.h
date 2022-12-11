#pragma once

#include <compare>
#include "core/chrono/tenor.h"
#include "adjustment_interface.h"

namespace egret::chrono {
// -----------------------------------------------------------------------------
//  [class] add_tenor
// -----------------------------------------------------------------------------
    class add_tenor : public adjustment_interface<add_tenor> {
    private:
        using this_type = add_tenor;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        add_tenor() = delete;
        constexpr add_tenor(const this_type&) noexcept = default;
        constexpr add_tenor(this_type&&) noexcept = default;

        explicit constexpr add_tenor(const chrono::tenor& tnr) noexcept
            : tnr_(tnr)
        {
        }

        constexpr add_tenor(std::int32_t count, chrono::tenor_unit unit) noexcept
            : tnr_(count, unit)
        {
        }

        constexpr this_type& operator =(const this_type&) noexcept = default;
        constexpr this_type& operator =(this_type&&) noexcept = default;

    // -------------------------------------------------------------------------
    //  adjustment behavior
    //
        constexpr auto operator()(const std::chrono::sys_days& d) const noexcept -> std::chrono::sys_days { return d + tnr_; }
        constexpr auto operator()(const std::chrono::sys_seconds& s) const noexcept -> std::chrono::sys_seconds { return s + tnr_; }

    // -------------------------------------------------------------------------
    //  compare  
    //
        constexpr auto operator<=>(const this_type&) const noexcept = default;

    // -------------------------------------------------------------------------
    //  get  
    //
        constexpr const chrono::tenor& tenor() const noexcept { return tnr_; }

    private:
        chrono::tenor tnr_;

    }; // class add_tenor

} // namespace egret::chrono
