#pragma once

#include <ranges>
#include <concepts>
#include <functional>
#include <list>
#include "iteration_result.h"

namespace egret::math::solver {
// -----------------------------------------------------------------------------
//  [struct] newton1d_iteration_log
// -----------------------------------------------------------------------------
    template <typename X>
    struct newton1d_iteration_log {
        X x;
        X f;
        X der;
    };

// -----------------------------------------------------------------------------
//  [fn] newton_raphson1d
// -----------------------------------------------------------------------------    
    /**
     * @brief Newton-raphson math::solver
     * @param f: x -> (function_value, derivative_value)
     * @param torerance: (prev_x, current_x) -> bool 
     * @param initial initial value of iteration
     * @param max_iter max number of the iteration
     * @return 
    */
    template <typename F, typename Tor, std::floating_point X>
        requires 
            std::is_invocable_r_v<std::pair<X, X>, const F&, const X&> &&
            std::predicate<const Tor&, const X&, const X&>
    auto newton_raphson1d(
        const F& f, const Tor& torerance, X initial,
        std::size_t max_iter = 1e3
    ) -> iteration_result<X, newton1d_iteration_log<X>>
    {
        iteration_result<X, newton1d_iteration_log<X>> result;
        try {
            X x = initial;
            std::size_t i = 0;
            for (; i != max_iter; ++i) {
                const std::pair<X, X> valuation_result = std::invoke(f, x);
                const auto& [fval, dval] = valuation_result;
                result.push(i, {.x = x, .f = fval, .der = dval});
                X new_x = x - fval / dval;
                if (!std::isfinite(new_x)) {
                    throw exception("Some invalid computation is detected. [iter={}, f={}, f'={}]", i, fval, dval);
                }
                if (std::invoke(torerance, x, new_x)) {
                    const std::pair<X, X> final_value = std::invoke(f, x);
                    result.set_result(new_x);
                    result.push(i + 1, {.x = new_x, .f = final_value.first, .der = final_value.second});
                    break;
                }
                x = std::move(new_x);
            }
            if (i == max_iter) {
                throw exception("newton_raphson1d does not converged.");
            }
        }
        catch (const exception& e) {
            result.set_error(e);
        }
        catch (const std::exception& e) {
            result.set_error(exception(e.what()).record_stacktrace());
        }
        catch (...) {
            result.set_error(exception("Newton-raphson 1d is failed with unexpected error.").record_stacktrace());
        }
        return result;
    }

//// -----------------------------------------------------------------------------
////  [fn] newton1d_6th_ord
//// -----------------------------------------------------------------------------    
//    /**
//     * @brief 1dim 6th order math::solver
//     * @details
//     *  Example 3.5 of
//     *  "Some higher-order modifications of Newtonfs method for solving nonlinear equations" by
//     *  YoonMee Ham, Changbum Chun and Sang-Gu Lee.
//     * 
//     * @param f: x -> f(x)
//     * @param der: x -> f'(x)
//     * @param torerance: (prev_x, current_x) -> bool 
//     * @param initial initial value of iteration
//     * @param max_iter max number of the iteration
//     * @return 
//    */
//    template <typename F, typename Der, typename Tor, std::floating_point X>
//        requires 
//            std::is_invocable_r_v<X, const F&, const X&> &&
//            std::is_invocable_r_v<X, const Der&, const X&> &&
//            std::predicate<const Tor&, const X&, const X&>
//    auto newton1d_6th_ord(
//        const F& f, const Der& der, const Tor& torerance, X initial,
//        std::size_t max_iter = 1e3
//    ) -> iteration_result<X, newton1d_iteration_log<X>>
//    {
//        iteration_result<X, newton1d_iteration_log<X>> result;
//        try {
//            X x = initial;
//            std::size_t i = 0;
//            for (; i != max_iter; ++i) {
//                const X fval_x = std::invoke(f, x);
//                const X dval_x = std::invoke(der, x);
//                result.push(i, {.x = x, .f = fval_x, .der = dval_x});
//
//                const X y = x - fval_x / dval_x;
//                const X dval_y = std::invoke(der, y);
//
//                const X z = x - 2. * fval_x * (1. / (dval_x + dval_y));    
//                const X fval_z = std::invoke(f, z);
//                const X new_x = z - 2 * dval_y * dval_y * fval_z / dval_x / (dval_x * dval_x - 4. * dval_x * dval_y + 5. * dval_y * dval_y);
//                if (!std::isfinite(new_x)) {
//                    throw exception("Some invalid computation is detected. [iter={}, f={}, f'={}, y={}, z={}]", i, fval_x, dval_x, y, z);
//                }
//                if (std::invoke(torerance, x, new_x)) {
//                    const X final_f = std::invoke(f, new_x);
//                    const X final_d = std::invoke(der, new_x);
//                    result.set_result(new_x);
//                    result.push(i + 1, {.x = new_x, .f = final_f, .der = final_d});
//                    break;
//                }
//                x = new_x;
//            }
//            if (i == max_iter) {
//                throw exception("newton_raphson1d does not converged.");
//            }
//        }
//        catch (const exception& e) {
//            result.set_error(e);
//        }
//        catch (const std::exception& e) {
//            result.set_error(exception(e.what()).record_stacktrace());
//        }
//        catch (...) {
//            result.set_error(exception("Newton-raphson 1d is failed with unexpected error.").record_stacktrace());
//        }
//        return result;        
//    }

} // namespace egret::math::solver