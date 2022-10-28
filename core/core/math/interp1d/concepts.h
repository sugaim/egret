#pragma once

#include <ranges>
#include <functional>
#include "core/concepts/range_of.h"
#include "core/utils/initialize_with.h"
#include "core/utils/update_with.h"

namespace egret::math::interp1d {
// -----------------------------------------------------------------------------
//  [struct] distance_definition
// -----------------------------------------------------------------------------
    template <typename X, typename Y = X>
    struct distance_definition {
        constexpr auto operator()(const X& from, const Y& to) const
            noexcept(noexcept(to - from))
            requires requires (const X& from, const Y& to) { {to - from} -> cpt::non_void; }
        {
            using diff_t = std::remove_cvref_t<decltype(to - from)>;
            if constexpr (std::is_integral_v<diff_t>) {
                return static_cast<double>(to - from);
            }
            else {
                return to - from;
            }
        }
    };

    template <typename C, typename D>
    struct distance_definition<std::chrono::time_point<C, D>, std::chrono::time_point<C, D>> {
        using time_point = std::chrono::time_point<C, D>;
        constexpr double operator()(const time_point& from, const time_point& to) const
        {
            const auto from_ = from.time_since_epoch();
            const auto to_ = to.time_since_epoch();
            return static_cast<double>((to_ - from_).count());
        }
    };

} // namespace egret::math::interp1d

namespace egret_detail::interp1d_impl {
    void grids(const auto&) = delete;

    class grids_t {
    public:
        template <typename F>
            requires 
                requires (const F& f) { { f.grids() } -> std::ranges::forward_range; }
        constexpr decltype(auto) operator()(const F& f) const
            noexcept(noexcept(f.grids()))
        {
            return f.grids();
        }

        template <typename F>
            requires 
                (!requires (const F& f) { { f.grids() } -> std::ranges::forward_range; }) &&
                requires (const F & f) { { grids(f) } -> std::ranges::forward_range; }
        constexpr decltype(auto) operator()(const F& f) const
            noexcept(noexcept(grids(f)))
        {
            return grids(f);
        }

    }; // class grids_t

    void values(const auto&) = delete;

    class values_t {
    public:
        template <typename F>
            requires 
                requires (const F& f) { { f.values() } -> std::ranges::forward_range; }
        constexpr decltype(auto) operator()(const F& f) const
            noexcept(noexcept(f.values()))
        {
            return f.values();
        }

        template <typename F>
            requires 
                (!requires (const F& f) { { f.values() } -> std::ranges::forward_range; }) &&
                requires (const F& f) { { values(f) } -> std::ranges::forward_range; }
        constexpr decltype(auto) operator()(const F& f) const
            noexcept(noexcept(values(f)))
        {
            return values(f);
        }
        
    }; // class values_t

    void distance(auto&&, auto&&) = delete;

    class distance_t {
    private:
        template <typename X, typename Y>
        using definition = egret::math::interp1d::distance_definition<X, Y>;

    public:
        template <typename X, typename Y>
            requires std::is_invocable_v<definition<X, Y>, const X&, const Y&>
        constexpr auto operator()(const X& from, const Y& to) const
            noexcept(std::is_nothrow_invocable_v<definition<X, Y>, const X&, const Y&>)
        {
            return definition<X, Y>{}(from, to);
        }
        template <typename X, typename Y>
            requires 
                (!std::is_invocable_v<definition<X, Y>, const X&, const Y&>) &&
                (std::is_invocable_v<definition<Y, X>, const X&, const Y&>) &&
                requires (std::invoke_result_t<definition<Y, X>, const X&, const Y&> inv) {
                    - inv;
                }
        constexpr auto operator()(const X& from, const Y& to) const
            noexcept(noexcept(- definition<Y, X>{}(to, from)))
        {
            return - definition<Y, X>{}(to, from);
        }

    }; // class distance_t 

} // namespace egret_detail::interp1d_impl

namespace egret::math::interp1d::inline cpo {
// -----------------------------------------------------------------------------
//  [cpo] grids
//  [cpo] values
// -----------------------------------------------------------------------------
    inline constexpr auto grids = egret_detail::interp1d_impl::grids_t {};
    inline constexpr auto values = egret_detail::interp1d_impl::values_t {};

// -----------------------------------------------------------------------------
//  [cpo] distance
// -----------------------------------------------------------------------------
    inline constexpr auto distance = egret_detail::interp1d_impl::distance_t {};

} // namespace egret::math::interp1d::inline cpo


namespace egret::math::interp1d {
// -----------------------------------------------------------------------------
//  [type] grids_t
//  [type] values_t
// -----------------------------------------------------------------------------
    template <typename F>
        requires requires (const F& f) { cpo::grids(f); }
    using grids_t = decltype(cpo::grids(std::declval<const F&>()));

    template <typename F>
        requires requires (const F& f) { cpo::values(f); }
    using values_t = decltype(cpo::values(std::declval<const F&>()));

// -----------------------------------------------------------------------------
//  [concept] distance_measurable_from
//  [concept] distance_measurable
// -----------------------------------------------------------------------------
    template <typename T, typename Base>
    concept distance_measurable_from = requires (const Base& from, const T& to) {
        { interp1d::distance(from, to) } -> cpt::non_void;
    };

    template <typename T>
    concept distance_measurable = distance_measurable_from<T, T>;

// -----------------------------------------------------------------------------
//  [type] distance_result_t
// -----------------------------------------------------------------------------
    template <typename X, typename Y = X>
        requires distance_measurable_from<X, Y>
    using distance_result_t = std::remove_cvref_t<
        decltype(interp1d::distance(std::declval<const X&>(), std::declval<const Y&>()))
    >;

// -----------------------------------------------------------------------------
//  [concept] relpos_computable_from
//  [concept] relpos_computable
// -----------------------------------------------------------------------------
    template <typename X, typename Base>
    concept relpos_computable_from =
        distance_measurable_from<X, Base> &&
        requires (const X& x, const Base& base, const X& base_end) {
            { interp1d::distance(base, x) / interp1d::distance(base, base_end) } -> cpt::non_void;
        };

    template <typename X>
    concept relpos_computable =
        relpos_computable_from<X, X>;

// -----------------------------------------------------------------------------
//  [type] relpos_t
// -----------------------------------------------------------------------------
    template <typename X, typename Base = X>
        requires relpos_computable_from<X, Base>
    using relpos_t = std::remove_cvref_t<decltype(
        interp1d::distance(std::declval<const Base&>(), std::declval<const X&>()) 
        / interp1d::distance(std::declval<const Base&>(), std::declval<const Base&>())
    )>;

} // namespace egret::math::interp1d

namespace egret::cpt {
// -----------------------------------------------------------------------------
//  [concept] interpolation1d
// -----------------------------------------------------------------------------
    template <typename F, typename X>
    concept interpolation1d = 
        std::is_invocable_v<const F&, const X&> &&
        requires (const F& f) {
            { math::interp1d::grids(f) } -> std::ranges::forward_range;
            { math::interp1d::values(f) } -> std::ranges::forward_range;
        };
        
// -----------------------------------------------------------------------------
//  [concept] interpolation1d_of
// -----------------------------------------------------------------------------
    template <typename F, typename X, typename Y>
    concept interpolation1d_of = 
        std::is_invocable_r_v<Y, const F&, const X&> &&
        requires (const F& f) {
            { math::interp1d::grids(f) } -> cpt::forward_range_of<X>;
            { math::interp1d::values(f) } -> cpt::forward_range_of<Y>;
        };

// -----------------------------------------------------------------------------
//  [concept] mutable_interpolation1d
// -----------------------------------------------------------------------------
    template <typename F, typename Xs, typename Ys>
    concept mutable_interpolation1d =
        std::ranges::forward_range<Xs> &&
        std::ranges::forward_range<Ys> &&
        interpolation1d_of<F, std::ranges::range_value_t<Xs>, std::ranges::range_value_t<Ys>> &&
        cpt::initializable_with<F, Xs, Ys> &&
        cpt::updatable_with<F, std::size_t, const std::ranges::range_reference_t<Ys>>;

} // namespace egret::cpt
