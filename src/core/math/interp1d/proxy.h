#pragma once

#include <utility>
#include "core/concepts/non_reference.h"
#include "core/concepts/non_void.h"
#include "core/math/der.h"
#include "core/math/integrate.h"
#include "concepts.h"

namespace egret::math::interp1d {
// -----------------------------------------------------------------------------
//  [class] proxy
// -----------------------------------------------------------------------------
    template <cpt::non_reference F>
    class proxy {
    private:
        using this_type = proxy;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        proxy() = delete;
        constexpr proxy(const this_type&) noexcept = default;
        constexpr proxy(this_type&&) noexcept = default;

        explicit proxy(const F& ref) noexcept : ref_(ref) {}
        proxy(F&&) = delete;

        constexpr this_type& operator =(const this_type&) noexcept = default;
        constexpr this_type& operator =(this_type&&) noexcept = default;

    // -------------------------------------------------------------------------
    //  interpolation behavior
    //
        template <typename X>
            requires requires (const F& f, const X& x) {
                { std::invoke(f, x) } -> cpt::non_void;
            }
        constexpr decltype(auto) operator()(const X& x) const
        {
            return std::invoke(ref_.get(), x);
        }

        constexpr decltype(auto) grids() const noexcept(noexcept(interp1d::grids(std::declval<const F&>())))
        { 
            return interp1d::grids(ref_.get());
        }

        constexpr decltype(auto) values() const noexcept(noexcept(interp1d::values(std::declval<const F&>())))
        { 
            return interp1d::values(ref_.get());
        }

    // -------------------------------------------------------------------------
    //  der, integrate
    //
        template <typename X>
            requires cpt::fst_ord_differentiable<const F&, const X&>
        constexpr decltype(auto) der1(const X& x) const
        {
            return math::der1(ref_.get(), x);
        }

        template <typename X>
            requires cpt::snd_ord_differentiable<const F&, const X&>
        constexpr decltype(auto) der2(const X& x) const
        {
            return math::der2(ref_.get(), x);
        }

        template <typename X>
            requires cpt::integrable<const F&, const X&>
        constexpr decltype(auto) integrate(const X& from, const X& to) const
        {
            return math::integrate(ref_.get(), from, to);
        }

    // -------------------------------------------------------------------------
    //  get  
    //
        constexpr const F& get() const noexcept { return ref_.get(); }
        constexpr operator const F&() const noexcept { return ref_.get(); }

    private:
        std::reference_wrapper<const F> ref_;

    }; // class proxy

// -----------------------------------------------------------------------------
//  [class] mutable_proxy
// -----------------------------------------------------------------------------
    template <cpt::non_reference F>
    class mutable_proxy {
    private:
        using this_type = mutable_proxy;
        static_assert(!std::is_const_v<F>, "Const object can not be held by mutable_proxy.");

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        mutable_proxy() = delete;
        constexpr mutable_proxy(const this_type&) noexcept = default;
        constexpr mutable_proxy(this_type&&) noexcept = default;

        explicit mutable_proxy(F& ref) noexcept : ref_(ref) {}
        mutable_proxy(F&&) = delete;

        constexpr this_type& operator =(const this_type&) noexcept = default;
        constexpr this_type& operator =(this_type&&) noexcept = default;

    // -------------------------------------------------------------------------
    //  interpolation behavior
    //
        template <typename X>
            requires requires (const F& f, const X& x) {
                { std::invoke(f, x) } -> cpt::non_void;
            }
        constexpr decltype(auto) operator()(const X& x) const
        {
            return std::invoke(ref_.get(), x);
        }

        constexpr decltype(auto) grids() const noexcept(noexcept(interp1d::grids(std::declval<const F&>())))
        { 
            return interp1d::grids(ref_.get());
        }

        constexpr decltype(auto) values() const noexcept(noexcept(interp1d::values(std::declval<const F&>())))
        { 
            return interp1d::values(ref_.get());
        }

    // -------------------------------------------------------------------------
    //  mutable_behavior
    //
        template <typename Y>
            requires cpt::updatable_with<F, std::size_t, Y>
        void update(std::size_t i, Y&& value)
        {
            util::update_with(ref_.get(), i, std::forward<Y>(value));
        }

        template <typename Xs, typename Ys>
            requires cpt::initializable_with<F, Xs, Ys>
        void initialize(Xs&& xs, Ys&& ys)
        {
            util::initialize_with(ref_.get(), std::forward<Xs>(xs), std::forward<Ys>(ys));
        }

    // -------------------------------------------------------------------------
    //  der, integrate
    //
        template <typename X>
            requires cpt::fst_ord_differentiable<const F&, const X&>
        constexpr decltype(auto) der1(const X& x) const
        {
            return math::der1(ref_.get(), x);
        }

        template <typename X>
            requires cpt::snd_ord_differentiable<const F&, const X&>
        constexpr decltype(auto) der2(const X& x) const
        {
            return math::der2(ref_.get(), x);
        }

        template <typename X>
            requires cpt::integrable<const F&, const X&>
        constexpr decltype(auto) integrate(const X& from, const X& to) const
        {
            return math::integrate(ref_.get(), from, to);
        }

    // -------------------------------------------------------------------------
    //  get  
    //
        constexpr F& get() noexcept { return ref_.get(); }
        constexpr const F& get() const noexcept { return ref_.get(); }

        constexpr operator F&() noexcept { return ref_.get(); }
        constexpr operator const F&() const noexcept { return ref_.get(); }

    private:
        std::reference_wrapper<F> ref_;

    }; // class mutable_proxy

} // namespace egret::math::interp1d
