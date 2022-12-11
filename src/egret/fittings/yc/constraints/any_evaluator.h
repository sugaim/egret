#pragma once

#include <map>
#include <memory>
#include "core/utils/maybe.h"
#include "egret/models/curves/concepts.h"
#include "egret/models/curves/any_yield_curve.h"
#include "concepts.h"

namespace egret::fit::yc {
// -----------------------------------------------------------------------------
//  [class] any_evaluator
// -----------------------------------------------------------------------------
    template <typename R, typename RateTag, cpt::yield_curve_r<R> Curve = model::any_yield_curve<R>>
    class any_evaluator {
    private:
        using this_type = any_evaluator;

        struct base {
            virtual ~base() = default;
            virtual R apply(const std::chrono::sys_days& vdt, const std::map<RateTag, Curve>& curves) const = 0;
            virtual std::type_index type() const noexcept = 0;
            virtual const void* pointer() const noexcept = 0;
        };

        template <typename C>
        struct concrete final : base {
            concrete(const C& obj) : obj_(obj) {}
            concrete(C&& obj) : obj_(std::move(obj)) {}

            R apply(const std::chrono::sys_days& vdt, const std::map<RateTag, Curve>& curves) const override { return yc::evaluate(obj_, vdt, curves); }
            std::type_index type() const noexcept override { return typeid(C); }
            const void* pointer() const noexcept override { return std::addressof(obj_);  }

            C obj_;
        };

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        any_evaluator() = delete;
        any_evaluator(const this_type&) noexcept = default;
        any_evaluator(this_type&&) noexcept = default;

        template <cpt::yield_curve_evaluator_r<R, RateTag, Curve> C>
        any_evaluator(C&& obj)
            : obj_(std::make_shared<concrete<std::remove_cvref_t<C>>>(std::forward<C>(obj)))
        {
        }

        this_type& operator =(const this_type&) noexcept = default;
        this_type& operator =(this_type&&) noexcept = default;

    // -------------------------------------------------------------------------
    //  yield_curve_evaluator behavior
    //
        R evaluate(const std::chrono::sys_days& vdt, const std::map<RateTag, Curve>& curves) const { return obj_->apply(vdt, curves); }

    // -------------------------------------------------------------------------
    //  type_erasure behavior
    //
        std::type_index type() const noexcept { return obj_->type(); }

        template <cpt::yield_curve_evaluator_r<R, RateTag, Curve> C>
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

    }; // class any_evaluator

} // namespace egret::fit::yc
