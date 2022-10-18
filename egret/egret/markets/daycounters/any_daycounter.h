#pragma once

#include <memory>
#include <typeindex>
#include "core/utils/maybe.h"
#include "concepts.h"

namespace egret::mkt {
// -----------------------------------------------------------------------------
//  [class] any_daycounter
// -----------------------------------------------------------------------------
    template <typename TimePoint = std::chrono::sys_days>
    class any_daycounter {
    private:
       using this_type = any_daycounter;

        struct base {
            virtual ~base() = default;
            virtual std::unique_ptr<const base> clone() const = 0;
            virtual double dcf(const TimePoint& from, const TimePoint& to) const = 0;
            virtual std::type_index type() const noexcept = 0;
            virtual const void* pointer() const noexcept = 0;
        };

        template <typename C>
        struct model final : base {
            model(C obj) noexcept(std::is_nothrow_move_constructible_v<C>): obj_(std::move(obj)) {}

            std::unique_ptr<const base> clone() const { return std::make_unique<model>(obj_); }
            double dcf(const TimePoint& from, const TimePoint& to) const override { return mkt::dcf(obj_, from, to); }
            std::type_index type() const noexcept override { return typeid(C); }
            const void* pointer() const noexcept override { return std::addressof(obj_); }

            C obj_;
        };

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        any_daycounter() = delete;
        any_daycounter(const this_type& other) : obj_(other.obj_->clone()) {}
        any_daycounter(this_type&&) noexcept = default;
        
        template <cpt::daycounter<TimePoint> DC>
        any_daycounter(DC&& dc)
            : obj_(std::make_unique<model<std::remove_cvref_t<DC>>>(std::forward<DC>(dc)))
        {
        }

        this_type& operator =(const this_type& other)
        {
            if (&other == this) {
                return *this;
            }
            obj_ = other.obj_->clone();
            return *this;
        }
        this_type& operator =(this_type&&) noexcept = default;

    // -------------------------------------------------------------------------
    //  daycounter behavior
    //
        double operator()(const TimePoint& from, const TimePoint& to) const { return obj_->dcf(from, to); }

    // -------------------------------------------------------------------------
    //  get
    //
        std::type_index type() const noexcept { return obj_->type(); }

        template <cpt::daycounter<TimePoint> DC>
        util::maybe<const DC&> as() const noexcept
        {
            if (this->type() == typeid(DC)) {
                return util::maybe<const DC&>(*reinterpret_cast<const DC*>(obj_->pointer()));
            }
            else {
                return util::maybe<const DC&>();
            }
        }
        
    private:
        std::unique_ptr<const base> obj_;

    }; // class any_daycounter
    
} // namespace egret::mkt