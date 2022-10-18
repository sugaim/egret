#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <ranges>
#include <typeindex>
#include <utility>
#include <expected>
#include "core/concepts/qualified.h"
#include "core/concepts/different_from.h"
#include "core/assertions/exception.h"
#include "core/utils/range_utils/vector_assign.h"
#include "core/utils/maybe.h"
#include "core/math/der.h"
#include "core/math/integrate.h"
#include "concepts.h"

namespace egret::math::interp1d {
// -----------------------------------------------------------------------------
//  [class] any
// -----------------------------------------------------------------------------
    template <typename X, typename Y>
    class any {
    private:
        using this_type = any;

        struct base {
            virtual ~base() = default;
            virtual Y call(const X& x) const = 0;
            virtual const std::vector<X>& grids() const noexcept = 0;
            virtual const std::vector<Y>& values() const noexcept = 0;
            virtual std::type_index type() const noexcept = 0;
            virtual const void* ptr() const noexcept = 0;
            virtual std::optional<Y> der1(const X& arg) const = 0;
            virtual std::optional<Y> der2(const X& arg) const = 0;
            virtual std::optional<Y> integrate(const X& from, const X& to) const = 0;
        };

        template <typename C>
        struct concrete final : base {
            concrete(C obj) 
                : obj_(std::move(obj)), grids_(), values_()
            {
                util::vector_assign(grids_, interp1d::grids(obj_));
                util::vector_assign(values_, interp1d::values(obj_));
            }
            Y call(const X& x) const override { return obj(x); }
            const std::vector<X>& grids() const noexcept override { return grids_; }
            const std::vector<Y>& values() const noexcept override { return values_;}
            std::type_index type() const noexcept override { return typeid(C); }
            const void* ptr() const noexcept override { return std::addressof(obj_); }
            std::optional<Y> der1(const X& arg) const override
            {
                if constexpr (cpt::fst_ord_differentiable_r<C, Y, X>) {
                    return math::der1(obj_, arg);
                }
                else {
                    return std::nullopt;
                }
            }
            std::optional<Y> der2(const X& arg) const override
            {
                if constexpr (cpt::snd_ord_differentiable_r<C, Y, X>) {
                    return math::der2(obj_, arg);
                }
                else {
                    return std::nullopt;
                }
            }
            std::optional<Y> integrate(const X& from, const X& to) const override
            {
                if constexpr (cpt::integrable_r<C, Y, X>) {
                    return math::integrate(obj_, from, to);
                }
                else {
                    return std::nullopt;
                }
            }

            C obj_;
            std::vector<X> grids_;
            std::vector<Y> values_;
        };

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtors and assigns
    //
        any() = delete;
        any(const this_type&) noexcept = default;
        any(this_type&&) noexcept = default;

        template <cpt::interpolation1d_of<X, Y> F>
            requires cpt::different_from<F, this_type>
        explicit any(F&& f)
            : obj_(std::make_shared<concrete<std::remove_cvref_t<F>>>(std::forward<F>(f)))
        {
        }

        this_type& operator =(const this_type&) noexcept = default;
        this_type& operator =(this_type&&) noexcept = default;

    // -------------------------------------------------------------------------
    //  interpolation behavior
    //
        Y operator()(const X& x) const { return obj_->call(x); }
        const std::vector<X>& grids() const noexcept { return obj_->grids(); }
        const std::vector<Y>& values() const noexcept { return obj_->values(); }

    // -------------------------------------------------------------------------
    //  der, integrate
    //
        Y der1(const X& x) const
        {
            auto result = obj_->der1(x);
            return result ? *std::move(result) : throw exception("Internal object does not supprot der1.").record_stacktrace();
        }

        Y der2(const X& x) const
        {
            auto result = obj_->der2(x);
            return result ? *std::move(result) : throw exception("Internal object does not supprot der2.").record_stacktrace();
        }

        Y integrate(const X& from, const X& to) const
        {
            auto result = obj_->integrate(from, to);
            return result ? *std::move(result) : throw exception("Internal object does not supprot integrate.").record_stacktrace();
        }

    // -------------------------------------------------------------------------
    //  type_erasure behavior
    //
        std::type_index type() const noexcept { return obj_->type(); }

        template <cpt::interpolation1d_of<X, Y> C>
        util::maybe<const C&> as() const noexcept
        {
            const auto type = this->type();
            return type == typeid(C) 
                ? util::maybe<const C&>(*reinterpret_cast<const C*>(obj_->ptr()))
                : util::maybe<const C&>();
        }

    private:
        std::shared_ptr<const base> obj_;

    }; // class any

// -----------------------------------------------------------------------------
//  [class] any_mutable
// -----------------------------------------------------------------------------
    template <std::ranges::forward_range Xs, std::ranges::forward_range Ys>
    class any_mutable {
    private:
        using this_type = any_mutable;
        using grid_type = std::ranges::range_value_t<Xs>;
        using value_type = std::ranges::range_value_t<Ys>;

        struct base {
            virtual ~base() = default;
            virtual std::shared_ptr<base> clone() const = 0;
            virtual value_type call(const grid_type& x) const = 0;
            virtual const std::vector<grid_type>& grids() const noexcept = 0;
            virtual const std::vector<value_type>& values() const noexcept = 0;
            virtual void initialize(const Xs& grids, const Ys& values) = 0;
            virtual void initialize(Xs&& grids, const Ys& values) = 0;
            virtual void initialize(const Xs& grids, Ys&& values) = 0;
            virtual void initialize(Xs&& grids, Ys&& values) = 0;
            virtual void update(std::size_t i, const value_type& value) = 0;
            virtual std::type_index type() const noexcept = 0;
            virtual const void* ptr() const noexcept = 0;
            virtual std::optional<value_type> der1(const grid_type& arg) const = 0;
            virtual std::optional<value_type> der2(const grid_type& arg) const = 0;
        };

        template <typename C>
        struct concrete final : base {
            concrete(C obj) : obj_(std::move(obj)), grids_(), values_() 
            { 
                this->set_vectors(); 
            }
            std::shared_ptr<base> clone() const override { return std::make_shared<concrete>(obj_); }
            value_type call(const grid_type& x) const override { return obj(x); }
            const std::vector<grid_type>& grids() const noexcept override { return grids_; }
            const std::vector<value_type>& values() const noexcept override { return values_;}
            void initialize(const Xs& grids, const Ys& values) override { util::initialize_with(obj_, grids, values); this->set_vectors(); }
            void initialize(Xs&& grids, const Ys& values) override { util::initialize_with(obj_, std::move(grids), values); this->set_vectors(); }
            void initialize(const Xs& grids, Ys&& values) override { util::initialize_with(obj_, grids, std::move(values)); this->set_vectors(); }
            void initialize(Xs&& grids, Ys&& values) override { util::initialize_with(obj_, std::move(grids), std::move(values)); this->set_vectors(); }
            void update(std::size_t i, const value_type& value) override
            {
                if (values_.size() <= i) {
                    throw exception("Updated out of range value. [size={}, tried={}]", values_.size(), i);
                }
                util::update_with(obj_, i, value);
                values_[i] = value;
            }
            std::type_index type() const noexcept override { return typeid(C); }
            const void* ptr() const noexcept override { return std::addressof(obj_); }
            std::optional<value_type> der1(const grid_type& arg) const override
            {
                if constexpr (cpt::fst_ord_differentiable_r<C, value_type, grid_type>) {
                    return std::nullopt;
                }
                else {
                    return math::der1(obj_, arg);
                }
            }
            std::optional<value_type> der2(const grid_type& arg) const override
            {
                if constexpr (cpt::snd_ord_differentiable_r<C, value_type, grid_type>) {
                    return std::nullopt;
                }
                else {
                    return math::der2(obj_, arg);
                }
            }

            void set_vectors()
            {
                util::vector_assign(grids_, interp1d::grids(obj_));
                util::vector_assign(values_, interp1d::values(obj_));
            }

            C obj_;
            std::vector<grid_type> grids_;
            std::vector<value_type> values_;
        };

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtors and assigns
    //
        any_mutable() = delete;
        any_mutable(const this_type&) noexcept = default;
        any_mutable(this_type&&) noexcept = default;

        template <cpt::mutable_interpolation1d<Xs, Ys> F>
            requires cpt::different_from<F, this_type>
        explicit any_mutable(F&& f)
            : obj_(std::make_shared<concrete<std::remove_cvref_t<F>>>(std::forward<F>(f)))
        {
        }

        this_type& operator =(const this_type&) noexcept = default;
        this_type& operator =(this_type&&) noexcept = default;

    // -------------------------------------------------------------------------
    //  mutable interpolation behavior
    //
        value_type operator()(const grid_type& x) const { return obj_->call(x); }
        const std::vector<grid_type>& grids() const noexcept { return obj_->grids(); }
        const std::vector<value_type>& values() const noexcept { return obj_->values(); }

        void update(std::size_t i, const value_type& value)
        {
            // copy on write
            std::shared_ptr<base> copied = obj_->clone();
            copied->update(i, value);
            obj_.swap(copied);
        }

        template <cpt::qualified<Xs> AXs, cpt::qualified<Ys> AYs>
        void initialize(AXs&& xs, AYs&& ys)
        {
            // copy on write
            std::shared_ptr<base> copied = obj_->clone();
            copied->initialize(std::forward<AXs>(xs), std::forward<AYs>(ys));
            obj_.swap(copied);            
        }
    
    // -------------------------------------------------------------------------
    //  type_erasure behavior
    //
        std::type_index type() const noexcept { return obj_->type(); }

        template <cpt::mutable_interpolation1d<Xs, Ys> C>
        util::maybe<const C&> as() const noexcept
        {
            const auto type = this->type();
            return type == typeid(C) 
                ? util::maybe<const C&>(*reinterpret_cast<const C*>(obj_->ptr()))
                : util::maybe<const C&>();
        }

    private:
        std::shared_ptr<base> obj_;

    }; // class any_mutable

} // namespace egret::math::interp1d
