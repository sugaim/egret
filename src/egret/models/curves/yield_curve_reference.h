#pragma once

#include <chrono>
#include <utility>
#include "concepts.h"

namespace egret::model {
// -----------------------------------------------------------------------------
//  [class] yield_curve_reference
// -----------------------------------------------------------------------------
    template <cpt::yield_curve C>
    class yield_curve_reference {
    private:
        using this_type = yield_curve_reference;

    public:
    // -------------------------------------------------------------------------
    //  ctor, dtor and assigns
    //
        yield_curve_reference() = delete;
        constexpr yield_curve_reference(const this_type&) noexcept = default;
        constexpr yield_curve_reference(this_type&&) noexcept = default;

        explicit constexpr yield_curve_reference(C& curve) noexcept
            : ref_(curve)
        {
        }

        constexpr this_type& operator =(const this_type&) noexcept = default;
        constexpr this_type& operator =(this_type&&) noexcept = default;

    // -------------------------------------------------------------------------
    //  forward_rate
    //
        constexpr auto forward_rate(const std::chrono::sys_days& from, const std::chrono::sys_days& to) const
        {
            return model::forward_rate(ref_.get(), from, to);
        }

    // -------------------------------------------------------------------------
    //  get
    //
        constexpr C& get() noexcept { return ref_; }
        constexpr const C& get() const noexcept { return ref_; }
        constexpr operator C&() noexcept { return ref_; }
        constexpr operator const C&() const noexcept { return ref_; }

    private:
        std::reference_wrapper<C> ref_;

    }; // class yield_curve_reference

    template <cpt::yield_curve C>
    constexpr yield_curve_reference<C> curve_ref(C& obj) noexcept
    {
        return yield_curve_reference<C>(obj);
    }
    template <cpt::yield_curve C>
    constexpr yield_curve_reference<const C> curve_cref(const C& obj) noexcept
    {
        return yield_curve_reference<const C>(obj);
    }

} // namespace egret::model
