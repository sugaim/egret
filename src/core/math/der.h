#pragma once

#include <concepts>
#include "core/concepts/non_void.h"

namespace egret_detail::der_impl {
    void der1(const auto&, const auto&) = delete;

    class der1_t {
    public:
        template <typename F, typename X>
            requires requires (const F& f, const X& x) { f.der1(x); }
        constexpr auto operator()(const F& f, const X& arg) const
        {
            return f.der1(arg);
        }

        template <typename F, typename X>
            requires 
                (!requires (const F& f, const X& x) { f.der1(x); }) &&
                requires (const F& f, const X& x) { der1(f, x); }
        constexpr auto operator()(const F& f, const X& arg) const
        {
            return der1(f, arg);
        }

    }; // class der1_t

} // namespace egret_detail::der_impl

namespace egret::math::inline cpo {
// -----------------------------------------------------------------------------
//  [cpo] der1
// -----------------------------------------------------------------------------
    inline constexpr auto der1 = egret_detail::der_impl::der1_t {};

} // namespace egret::math::inline cpo

namespace egret::cpt {
// -----------------------------------------------------------------------------
//  [concept] fst_ord_differentiable
//  [concept] fst_ord_differentiable_r
// -----------------------------------------------------------------------------
    template <typename F, typename X = double>
    concept fst_ord_differentiable = requires (const F & f, const X & x) {
        {math::der1(f, x)} -> cpt::non_void;
    };
    
    template <typename F, typename Y, typename X = double>
    concept fst_ord_differentiable_r = requires (const F & f, const X & x) {
        {math::der1(f, x)} -> std::convertible_to<Y>;
    };

} // namespace egret::cpt

namespace egret::math {
// -----------------------------------------------------------------------------
//  [type] der1_result_t
// -----------------------------------------------------------------------------
    template <typename F, typename X = double>
        requires cpt::fst_ord_differentiable<F, X>
    using der1_result_t = std::invoke_result_t<decltype(der1), const F&, const X&>;

} // namespace egret::math

namespace egret_detail::der_impl {
    void der2(const auto&, const auto&) = delete;

    class der2_t {
    public:
        template <typename F, typename X>
            requires requires (const F& f, const X& x) { f.der2(x); }
        constexpr auto operator()(const F& f, const X& arg) const
        {
            return f.der2(arg);
        }

        template <typename F, typename X>
            requires 
                (!requires (const F& f, const X& x) { f.der2(x); }) &&
                requires (const F& f, const X& x) { der2(f, x); }
        constexpr auto operator()(const F& f, const X& arg) const
        {
            return der2(f, arg);
        }

    }; // class der1_t

} // namespace egret_detail::der_impl

namespace egret::math::inline cpo {
// -----------------------------------------------------------------------------
//  [cpo] der2
// -----------------------------------------------------------------------------
    inline constexpr auto der2 = egret_detail::der_impl::der2_t {};

} // namespace egret::math::inline cpo

namespace egret::cpt {
// -----------------------------------------------------------------------------
//  [concept] snd_ord_differentiable
//  [concept] snd_ord_differentiable_r
// -----------------------------------------------------------------------------
    template <typename F, typename X = double>
    concept snd_ord_differentiable = requires (const F & f, const X & x) {
        {math::der2(f, x)} -> cpt::non_void;
    };
    
    template <typename F, typename Y, typename X = double>
    concept snd_ord_differentiable_r = requires (const F & f, const X & x) {
        {math::der2(f, x)} -> std::convertible_to<Y>;
    };

} // namespace egret::cpt

namespace egret::math {
// -----------------------------------------------------------------------------
//  [type] der2_result_t
// -----------------------------------------------------------------------------
    template <typename F, typename X = double>
        requires cpt::snd_ord_differentiable<F, X>
    using der2_result_t = std::invoke_result_t<decltype(der2), const F&, const X&>;

} // namespace egret::math
