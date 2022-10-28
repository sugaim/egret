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

        template <std::ranges::forward_range Xs, std::ranges::forward_range Ys> friend class any_mutable;

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

        template <typename C, typename T, bool for_grid>
        struct vector_provider {
            const std::vector<T>& get(const C&) const noexcept { return vector_; }
            template <typename Rng> void assign_if_non_member(Rng&& rng) { util::vector_assign(vector_, std::forward<Rng>(rng)); }
            template <typename U> void update(std::size_t i, U&& value) { vector_[i] = std::forward<U>(value); }
            std::vector<T> vector_;
        };

        template <typename C>
            requires requires (const C& obj) { {interp1d::grids(obj)} noexcept -> std::convertible_to<const std::vector<X>&>; }
        struct vector_provider<C, X, true> {
            const std::vector<X>& get(const C& obj) const noexcept { return interp1d::grids(obj); }
            template <typename Rng> void assign_if_non_member(Rng&&) noexcept {}
            template <typename U> void update(std::size_t, U&&) noexcept {}
        };

        template <typename C>
            requires requires (const C& obj) { {interp1d::values(obj)} noexcept -> std::convertible_to<const std::vector<Y>&>; }
        struct vector_provider<C, Y, false> {
            const std::vector<Y>& get(const C& obj) const noexcept { return interp1d::values(obj); }
            template <typename Rng> void assign_if_non_member(Rng&&) noexcept {}
            template <typename U> void update(std::size_t, U&&) noexcept {}
        };

        template <typename C>
        struct concrete final : base {
            concrete(C obj) 
                : obj_(std::move(obj)), grids_provider_(), values_provider_()
            {
                grids_provider_.assign_if_non_member(interp1d::grids(obj_));
                values_provider_.assign_if_non_member(interp1d::values(obj_));
            }
            Y call(const X& x) const override { return obj_(x); }
            const std::vector<X>& grids() const noexcept override { return grids_provider_.get(obj_); }
            const std::vector<Y>& values() const noexcept override { return values_provider_.get(obj_);}
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
            vector_provider<C, X, true> grids_provider_;
            vector_provider<C, Y, false> values_provider_;
        };

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtors and assigns
    //
    private:
        any() noexcept = default;

    public:
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
        std::shared_ptr<base> obj_;

    }; // class any

    template <typename F>
    any(F) -> any<std::ranges::range_value_t<grids_t<F>>, std::ranges::range_value_t<values_t<F>>>;

// -----------------------------------------------------------------------------
//  [class] any_mutable
// -----------------------------------------------------------------------------
    template <std::ranges::forward_range Xs, std::ranges::forward_range Ys>
    class any_mutable final : public any<std::ranges::range_value_t<Xs>, std::ranges::range_value_t<Ys>> {
    private:
        using this_type = any_mutable;
        using grid_type = std::ranges::range_value_t<Xs>;
        using value_type = std::ranges::range_value_t<Ys>;
        using super_type = any<grid_type, value_type>;

        struct mutable_base : super_type::base {
            virtual std::shared_ptr<mutable_base> clone() const = 0;
            virtual void initialize(const Xs& grids, const Ys& values) = 0;
            virtual void initialize(Xs&& grids, const Ys& values) = 0;
            virtual void initialize(const Xs& grids, Ys&& values) = 0;
            virtual void initialize(Xs&& grids, Ys&& values) = 0;
            virtual void update(std::size_t i, const value_type& value) = 0;
        };

        template <typename C>
        struct mutable_concrete final : mutable_base {
            using internal_type = super_type::template concrete<C>;
            mutable_concrete(C obj) : obj_(std::move(obj)) {}

            value_type call(const grid_type& x) const override { return obj_.call(x); }
            const std::vector<grid_type>& grids() const noexcept override { return obj_.grids(); }
            const std::vector<value_type>& values() const noexcept override { return obj_.values();}
            std::type_index type() const noexcept override { return obj_.type(); }
            const void* ptr() const noexcept override { return obj_.ptr(); }
            std::optional<value_type> der1(const grid_type& arg) const override { return obj_.der1(arg); }
            std::optional<value_type> der2(const grid_type& arg) const override { return obj_.der2(arg); }
            std::optional<value_type> integrate(const grid_type& from, const grid_type& to) const override { return obj_.integrate(from, to); }

            std::shared_ptr<mutable_base> clone() const override { return std::make_shared<mutable_concrete>(obj_.obj_); }
            void initialize(const Xs& grids, const Ys& values) override { util::initialize_with(obj_.obj_, grids, values); this->set_vectors(); }
            void initialize(Xs&& grids, const Ys& values) override { util::initialize_with(obj_.obj_, std::move(grids), values); this->set_vectors(); }
            void initialize(const Xs& grids, Ys&& values) override { util::initialize_with(obj_.obj_, grids, std::move(values)); this->set_vectors(); }
            void initialize(Xs&& grids, Ys&& values) override { util::initialize_with(obj_.obj_, std::move(grids), std::move(values)); this->set_vectors(); }
            void update(std::size_t i, const value_type& value) override
            {
                if (this->values().size() <= i) {
                    throw exception("Updated out of range value. [size={}, tried={}]", this->values().size(), i);
                }
                util::update_with(obj_.obj_, i, value);
                obj_.values_provider_.update(i, value);
            }

            void set_vectors()
            {
                obj_.grids_provider_.assign_if_non_member(interp1d::grids(obj_.obj_));
                obj_.values_provider_.assign_if_non_member(interp1d::values(obj_.obj_));
            }

            internal_type obj_;
        };

        void internal_clone_for_write()
        {
            const mutable_base* ptr = static_cast<const mutable_base*>(super_type::obj_.get());
            super_type::obj_ = ptr->clone();
        }

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
            : super_type()
        {
            super_type::obj_ = std::make_shared<mutable_concrete<std::remove_cvref_t<F>>>(std::forward<F>(f));
        }

        this_type& operator =(const this_type&) noexcept = default;
        this_type& operator =(this_type&&) noexcept = default;

    // -------------------------------------------------------------------------
    //  mutable interpolation behavior
    //
        void update(std::size_t i, const value_type& value)
        {
            if (1 < super_type::obj_.use_count()) {
                // copy on write
                this->internal_clone_for_write();
            }
            mutable_base* ptr = static_cast<mutable_base*>(super_type::obj_.get());
            ptr->update(i, value);
        }

        template <cpt::qualified<Xs> AXs, cpt::qualified<Ys> AYs>
        void initialize(AXs&& xs, AYs&& ys)
        {
            if (1 < super_type::obj_.use_count()) {
                // copy on write
                this->internal_clone_for_write();
            }
            mutable_base* ptr = static_cast<mutable_base*>(super_type::obj_.get());
            ptr->initialize(std::forward<AXs>(xs), std::forward<AYs>(ys));
        }
    
    }; // class any_mutable

    template <typename F>
    any_mutable(F) -> any_mutable<
        std::vector<std::ranges::range_value_t<grids_t<F>>>, 
        std::vector<std::ranges::range_value_t<values_t<F>>>
    >;

} // namespace egret::math::interp1d
