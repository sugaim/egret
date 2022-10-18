#pragma once

#include <tuple>
#include "core/concepts/qualified.h"
#include "core/type_traits/special_functions_properties.h"
#include "core/utils/member_data.h"
#include "concepts.h"

namespace egret_detail::bmped_crv_impl {
    template <typename T>
    struct is_tuple : std::false_type {};

    template <typename ...Ts>
    struct is_tuple<std::tuple<Ts...>> : std::true_type {};

    template <typename T>
    inline constexpr auto is_tuple_v = is_tuple<std::remove_cvref_t<T>>::value;

    template <typename T>
    constexpr decltype(auto) wrap_as_tuple(T&& obj)
    {
        if constexpr (is_tuple_v<T>) {
            return std::forward<T>(obj);
        }
        else {
            return std::forward_as_tuple(std::forward<T>(obj));
        }
    }

    template <typename ...Ts>
    constexpr auto make_tuple(Ts&& ...ts)
    {
        return std::tuple_cat(bmped_crv_impl::wrap_as_tuple(std::forward<Ts>(ts))...);
    }

} // namespace egret_detail::bmped_crv_impl

namespace egret::model {
// -----------------------------------------------------------------------------
//  [fn] forward_rate_adjustment
// -----------------------------------------------------------------------------
    template <cpt::yield_curve_adjustment ...Adjs>
    constexpr auto forward_rate_adjustment(
        const std::tuple<Adjs...>& adj,
        const std::chrono::sys_days& from,
        const std::chrono::sys_days& to
    )
    {
        using result_t = std::common_type_t<double, forward_rate_adjustment_t<Adjs>...>;
        const auto calculator = [&from, &to](const auto& ...adjs) {
            return (
                static_cast<result_t>(0.) + ... 
                + static_cast<result_t>(model::forward_rate_adjustment(adjs, from, to))
            );
        };
        return std::apply(calculator, adj);
    }

// -----------------------------------------------------------------------------
//  [class] bumped_yield_curve
// -----------------------------------------------------------------------------
    template <cpt::yield_curve C, cpt::yield_curve_adjustment Adj>
    class bumped_yield_curve {
    private:
        using this_type = bumped_yield_curve;
        using spfn_props = special_function_properties<util::member_data<C>, Adj>;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        bumped_yield_curve() = delete;
        constexpr bumped_yield_curve(const this_type&)
            noexcept(spfn_props::are_nothrow_copy_constructible_v)
            requires spfn_props::are_copy_constructible_v = default;
        constexpr bumped_yield_curve(this_type&&)
            noexcept(spfn_props::are_nothrow_move_constructible_v)
            requires spfn_props::are_move_constructible_v = default;

        template <cpt::qualified<C> AC, cpt::qualified<Adj> AAdj>
        constexpr bumped_yield_curve(AC&& crv, AAdj&& adj)
            : curve_(std::forward<AC>(crv)), adj_(std::forward<AAdj>(adj))
        {
        }

        constexpr this_type& operator =(const this_type&)
            noexcept(spfn_props::are_nothrow_copy_assignable_v)
            requires spfn_props::are_copy_assignable_v = default;
        constexpr this_type& operator =(this_type&&)
            noexcept(spfn_props::are_nothrow_move_assignable_v)
            requires spfn_props::are_move_assignable_v = default;

    // -------------------------------------------------------------------------
    //  yield_curve behavior
    //
        auto forward_rate(const std::chrono::sys_days& from, const std::chrono::sys_days& to) const
        {
            auto base = model::forward_rate(curve_.get(), from, to);
            auto adj = model::forward_rate_adjustment(adj_, from, to);
            return std::move(base) + std::move(adj);
        }

    // -------------------------------------------------------------------------
    //  get
    //
        const C& underlying_curve() const noexcept { return curve_.get(); }
        const Adj& adjustment() const noexcept { return adj_; }

    private:
        util::member_data<C> curve_;
        Adj adj_;

    }; // class bumped_yield_curve

    template <cpt::yield_curve C, cpt::yield_curve_adjustment Adj>
    bumped_yield_curve(C, Adj) -> bumped_yield_curve<C, Adj>;

    template <cpt::yield_curve C, cpt::yield_curve_adjustment Adj>
    bumped_yield_curve(std::reference_wrapper<C>, Adj) -> bumped_yield_curve<C&, Adj>;


// -----------------------------------------------------------------------------
//  [fn] make_bumped
// -----------------------------------------------------------------------------
    template <cpt::yield_curve C, cpt::yield_curve_adjustment ...Adjs>
    constexpr auto make_bumped(C&& curve, Adjs&& ...adjs)
    {
        if constexpr (sizeof...(Adjs) == 1) {
            return bumped_yield_curve(std::forward<C>(curve), std::forward<Adjs>(adjs)...);
        }
        else {
            return bumped_yield_curve(
                std::forward<C>(curve),
                egret_detail::bmped_crv_impl::make_tuple(std::forward<Adjs>(adjs)...)
            );
        }
    }

    template <cpt::yield_curve C, cpt::yield_curve_adjustment ...Adjs>
    constexpr decltype(auto) make_bumped(std::reference_wrapper<C> curve, Adjs&& ...adjs)
    {
        if constexpr (sizeof...(Adjs) == 1) {
            return bumped_yield_curve(curve, std::forward<Adjs>(adjs)...);
        }
        else {
            return bumped_yield_curve(
                curve,
                egret_detail::bmped_crv_impl::make_tuple(std::forward<Adjs>(adjs)...)
            );
        }
    }

} // namespace egret::model
