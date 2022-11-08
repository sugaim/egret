#pragma once

#include <map>
#include "core/concepts/non_void.h"
#include "egret/models/curves/concepts.h"
#include "egret/models/curves/any_yield_curve.h"

namespace egret_detail::ycc_impl {
    void evaluate(auto&&, auto&&, auto&&) = delete;

    class evaluate_t {
    public:
        template <typename T, typename Tag, egret::cpt::yield_curve Curve>
            requires 
                requires (const T& evaluator, const std::chrono::sys_days& vdt, const std::map<Tag, Curve>& curves) {
                    { evaluator.evaluate(vdt, curves) } -> egret::cpt::non_void;
                } ||
                requires (const T& evaluator, const std::chrono::sys_days& vdt, const std::map<Tag, Curve>& curves) {
                    { evaluate(evaluator, vdt, curves) } -> egret::cpt::non_void;
                }
        auto operator()(const T& evaluator, const std::chrono::sys_days& vdt, const std::map<Tag, Curve>& curves) const
        {
            constexpr bool has_mf = requires (const T& e, const std::chrono::sys_days& v, const std::map<Tag, Curve>& c) {
                { e.evaluate(v, c) } -> egret::cpt::non_void;
            };
            if constexpr (has_mf) {
                return evaluator.evaluate(vdt, curves);
            }
            else {
                return evaluate(evaluator, vdt, curves);
            }
        }

    }; // class evaluate_t
    
} // namespace egret_detail::ycc_impl

namespace egret::fit::yc::inline cpo {
// -----------------------------------------------------------------------------
//  [cpo] evaluate
// -----------------------------------------------------------------------------
    inline constexpr auto evaluate = egret_detail::ycc_impl::evaluate_t {};

} // namespace egret::fit::yc::inline cpo

namespace egret::cpt {
// -----------------------------------------------------------------------------
//  [concept] yield_curve_evaluator
// -----------------------------------------------------------------------------
    template <typename T, typename RateTag, typename Curve>
    concept yield_curve_evaluator =
        cpt::yield_curve<Curve> &&
        requires (const T& evaluator, const std::chrono::sys_days& vdt, const std::map<RateTag, Curve>& curves) {
            { fit::yc::evaluate(evaluator, vdt, curves) } -> cpt::non_void;
        };

// -----------------------------------------------------------------------------
//  [concept] yield_curve_evaluator_r
// -----------------------------------------------------------------------------
    template <typename T, typename R, typename RateTag, typename Curve>
    concept yield_curve_evaluator_r =
        yield_curve_evaluator<T, RateTag, Curve> &&
        requires (const T& evaluator, const std::chrono::sys_days& vdt, const std::map<RateTag, Curve>& curves) {
            { fit::yc::evaluate(evaluator, vdt, curves) } -> std::convertible_to<R>;
        };

} // namespace egret::cpt

namespace egret::fit::yc {
// -----------------------------------------------------------------------------
//  [type] evaluate_result_t
// -----------------------------------------------------------------------------
    template <typename T, typename RateTag, typename Curve>
        requires cpt::yield_curve_evaluator<T, RateTag, Curve>
    using evaluate_result_t = decltype(evaluate(
        std::declval<const T&>(), 
        std::chrono::sys_days {}, 
        std::declval<const std::map<RateTag, Curve>&>()
    ));

} // namespace egret::fit::yc