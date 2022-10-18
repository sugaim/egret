#pragma once

#include <memory>
#include <chrono>
#include <typeindex>
#include <variant>
#include "core/concepts/non_reference.h"
#include "concepts.h"

namespace egret::model {
// -----------------------------------------------------------------------------
//  [class] shared_yield_curve
// -----------------------------------------------------------------------------
    template <cpt::yield_curve C>
        requires cpt::non_reference<C>
    class shared_yield_curve {
    private:
        using this_type = shared_yield_curve;
        template <typename> friend class shared_yield_curve;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        shared_yield_curve() = delete;
        constexpr shared_yield_curve(const this_type&) noexcept = default;
        constexpr shared_yield_curve(this_type&&) noexcept = default;

        template <typename ...Args>
            requires std::is_constructible_v<C, Args...>
        explicit constexpr shared_yield_curve(Args&& ...args)
            : obj_(std::make_shared<C>(std::forward<Args>(args)...))
        {
        }

        template <std::derived_from<C> D>
        constexpr shared_yield_curve(shared_yield_curve<D> curve)
            noexcept
            : obj_(curve.obj_)
        {
        }

        constexpr this_type& operator =(const this_type&) noexcept = default;
        constexpr this_type& operator =(this_type&&) noexcept = default;

    // -------------------------------------------------------------------------
    //  yield curve behavior
    //
        constexpr auto forward_rate(const std::chrono::sys_days& from, const std::chrono::sys_days& to) const
        {
            return model::forward_rate(*obj_, from, to);
        }

    // -------------------------------------------------------------------------
    //  get
    //
        constexpr const C& get() const noexcept { return obj_; }
        constexpr operator const C&() const noexcept { return this->get(); }
    
    private:
        std::shared_ptr<const C> obj_;
        
    }; // class shared_yield_curve

} // namespace egret::model
