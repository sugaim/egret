#pragma once

#include <memory>
#include <chrono>
#include <typeindex>
#include <variant>
#include "core/concepts/different_from.h"
#include "core/utils/maybe.h"
#include "concepts.h"

namespace egret::model {
// -----------------------------------------------------------------------------
//  [class] any_yield_curve
// -----------------------------------------------------------------------------
    template <typename R>
    class any_yield_curve {
    private:
        using this_type = any_yield_curve;

        struct base {
            virtual ~base() = default;
            virtual R forward_rate(const std::chrono::sys_days& from, const std::chrono::sys_days& to) const = 0;
            virtual std::type_index type() const noexcept = 0;
            virtual const void* pointer() const noexcept = 0;
        };

        template <typename C>
        struct concrete final : base {
            concrete(C obj) noexcept(std::is_nothrow_move_constructible_v<C>) : obj_(std::move(obj)) {}

            R forward_rate(const std::chrono::sys_days& from, const std::chrono::sys_days& to) const override
            { 
                return model::forward_rate(obj_, from, to); 
            }

            std::type_index type() const noexcept { return typeid(obj_); }
            const void* pointer() const noexcept { return std::addressof(obj_); }

            C obj_;
        };

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        any_yield_curve() = delete;
        any_yield_curve(const this_type&) noexcept = default;
        any_yield_curve(this_type&&) noexcept = default;

        template <cpt::yield_curve_r<R> C>
            requires 
                cpt::different_from<C, this_type> &&
                std::is_constructible_v<std::remove_cvref_t<C>, C>
        any_yield_curve(C&& curve)
            : obj_(std::make_shared<concrete<std::remove_cvref_t<C>>>(std::forward<C>(curve)))
        {
        }

        template <cpt::yield_curve_r<R> C, typename ...Args>
            requires 
                cpt::different_from<C, this_type> &&
                std::is_constructible_v<C, Args...> &&
                std::is_same_v<C, std::remove_cvref_t<C>>
        any_yield_curve(std::in_place_type_t<C>, Args&& ...rgs)
            : obj_(std::make_unique<concrete<C>>(std::forward<Args>(args)...))
        {
        }

        this_type& operator =(const this_type&) noexcept = default;
        this_type& operator =(this_type&&) noexcept = default;

    // -------------------------------------------------------------------------
    //  yield curve behavior
    //
        R forward_rate(const std::chrono::sys_days& from, const std::chrono::sys_days& to) const
        {
            return obj_->forward_rate(from, to);
        }

    // -------------------------------------------------------------------------
    //  type_erasure behavior
    //
        std::type_index type() const noexcept { return obj_->type(); }

        template <cpt::yield_curve_r<R> C>
        util::maybe<const C&> as() const noexcept
        {
            if (this->type() == typeid(C)) {
                return util::maybe<const C&>(*reinterpret_cast<const C*>(obj_->pointer()));
            }
            else {
                return util::maybe<const C&>();
            }
        }
        
    private:
        std::shared_ptr<const base> obj_;
        
    }; // class any_yield_curve

} // namespace egret::model
