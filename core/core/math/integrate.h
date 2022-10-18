#pragma once

#include "core/concepts/non_void.h"

namespace egret_detail::integral_impl {
    void integrate(auto&&, auto&&, auto&&) = delete;

    template <typename F, typename X>
    concept integrable_mf = requires (const F& f, const X& from, const X& to) {
        { f.integrate(from, to) } -> egret::cpt::non_void;
    };

    template <typename F, typename X>
    concept integrable_adl = requires (const F& f, const X& from, const X& to) {
        { integrate(f, from, to) } -> egret::cpt::non_void;
    };

    class integrate_t {
    public:
        template <typename F, typename X>
            requires integrable_mf<F, X> || integrable_adl<F, X>
        constexpr auto operator()(const F& f, const X& from, const X& to) const
        {
            if constexpr (integrable_mf<F, X>) {
                return f.integrate(from, to);
            }
            else {
                return integrate(f, from, to);
            }
        }                

    }; // class integrate_t

} // namespace egret_detail::integral_impl

namespace egret::math::inline cpo {
// -----------------------------------------------------------------------------
//  [cpo] integrate
// -----------------------------------------------------------------------------
    inline constexpr auto integrate = egret_detail::integral_impl::integrate_t {};

} // namespace egret::math::inline cpo

namespace egret::cpt {
// -----------------------------------------------------------------------------
//  [concept] integrable
//  [concept] integrable_r
// -----------------------------------------------------------------------------
    template <typename F, typename X = double>
    concept integrable = requires (const F& f, const X& from, const X& to) {
        { math::integrate(f, from, to) } -> cpt::non_void;
    };

    template <typename F, typename Y, typename X = double>
    concept integrable_r = requires (const F& f, const X& from, const X& to) {
        { math::integrate(f, from, to) } -> std::convertible_to<Y>;
    };

} // namespace egret::cpt

namespace egret::math {
// -----------------------------------------------------------------------------
//  [type] integrate_result_t
// -----------------------------------------------------------------------------
    template <typename F, typename X = double>
        requires cpt::integrable<F, X>
    using integrate_result_t = std::invoke_result_t<decltype(math::integrate), const F&, const X&, const X&>;

} // namespace egret::math
