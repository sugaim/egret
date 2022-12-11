#pragma once

#include <chrono>
#include "core/concepts/non_void.h"

namespace egret_detail::fwdrt_impl {
    void forward_rate(auto&&, auto&&) = delete;

    template <typename C, typename TimePoint>
    concept has_forward_rate_mf = requires (const C& crv, const TimePoint& from, const TimePoint& to) {
        { crv.forward_rate(from, to) } -> egret::cpt::non_void;
    };

    template <typename C, typename TimePoint>
    concept has_forward_rate_adl = requires (const C& crv, const TimePoint& from, const TimePoint& to) {
        { forward_rate(crv, from, to) } -> egret::cpt::non_void;
    };

    class forward_rate_t {
    public:
        template <typename C, typename TimePoint>
            requires has_forward_rate_mf<C, TimePoint> || has_forward_rate_adl<C, TimePoint>
        constexpr auto operator()(const C& crv, const TimePoint& from, const TimePoint& to) const
        {
            if constexpr (has_forward_rate_mf<C, TimePoint>) {
                return crv.forward_rate(from, to);
            }
            else {
                return forward_rate(crv, from, to);
            }
        }
    };

    void forward_rate_adjustment(auto&&, auto&&, auto&&) = delete;

    template <typename Adj, typename TimePoint>
    concept has_forward_rate_adjustment_mf = requires (const Adj& adj, const TimePoint& from, const TimePoint& to) {
        { adj.forward_rate_adjustment(from, to) } -> egret::cpt::non_void;
    };

    template <typename Adj, typename TimePoint>
    concept has_forward_rate_adjustment_adl = requires (const Adj& adj, const TimePoint& from, const TimePoint& to) {
        { forward_rate_adjustment(adj, from, to) } -> egret::cpt::non_void;
    };

    class forward_rate_adjustment_t {
    public:
        template <typename Adj, typename TimePoint>
            requires has_forward_rate_adjustment_mf<Adj, TimePoint> || has_forward_rate_adjustment_adl<Adj, TimePoint>
        constexpr auto operator()(const Adj& adj, const TimePoint& from, const TimePoint& to) const
        {
            if constexpr (has_forward_rate_adjustment_mf<Adj, TimePoint>) {
                return adj.forward_rate_adjustment(from, to);
            }
            else {
                return forward_rate_adjustment(adj, from, to);
            }
        }
    };
    
} // namespace egret_detail::fwdrt_impl

namespace egret::model::inline cpo {
// -----------------------------------------------------------------------------
//  [cpo] forward_rate
//  [cpo] forward_rate_adjustment
// -----------------------------------------------------------------------------
    inline constexpr auto forward_rate = egret_detail::fwdrt_impl::forward_rate_t {};
    inline constexpr auto forward_rate_adjustment = egret_detail::fwdrt_impl::forward_rate_adjustment_t {};

} // namespace egret::model::inline cpo

namespace egret::cpt {
// -----------------------------------------------------------------------------
//  [concept] yield_curve
//  [concept] yield_curve_r
// -----------------------------------------------------------------------------
    template <typename C>
    concept yield_curve = requires (
        const C& curve, 
        const std::chrono::sys_days& from, 
        const std::chrono::sys_days& to
    ) {
        { model::forward_rate(curve, from, to) } -> cpt::non_void;
    };

    template <typename C, typename R>
    concept yield_curve_r = yield_curve<C> && requires (
        const C& curve, 
        const std::chrono::sys_days& from, 
        const std::chrono::sys_days& to
    ) {
        { model::forward_rate(curve, from, to) } -> std::convertible_to<R>;
    };
    
// -----------------------------------------------------------------------------
//  [concept] yield_curve_adjustment
//  [concept] yield_curve_adjustment_r
// -----------------------------------------------------------------------------
    template <typename Adj>
    concept yield_curve_adjustment = requires (
        const Adj& adj, 
        const std::chrono::sys_days& from, 
        const std::chrono::sys_days& to
    ) {
        { model::forward_rate_adjustment(adj, from, to) } -> cpt::non_void;
    };

    template <typename Adj, typename R>
    concept yield_curve_adjustment_r = yield_curve_adjustment<Adj> && requires (
        const Adj& adj, 
        const std::chrono::sys_days& from, 
        const std::chrono::sys_days& to
    ) {
        { model::forward_rate_adjustment(adj, from, to) } -> std::convertible_to<R>;
    };
    
} // namespace egret::cpt

namespace egret::model {
// -----------------------------------------------------------------------------
//  [type] forward_rate_t
// -----------------------------------------------------------------------------
    template <cpt::yield_curve C>
    using forward_rate_t = std::remove_cvref_t<std::invoke_result_t<
        decltype(cpo::forward_rate),
        const C&, const std::chrono::sys_days&, const std::chrono::sys_days&
    >>;

// -----------------------------------------------------------------------------
//  [type] forward_rate_adjustment_t
// -----------------------------------------------------------------------------
    template <cpt::yield_curve_adjustment Adj>
    using forward_rate_adjustment_t = std::remove_cvref_t<std::invoke_result_t<
        decltype(cpo::forward_rate_adjustment),
        const Adj&, const std::chrono::sys_days&, const std::chrono::sys_days&
    >>;

} // namespace egret::model