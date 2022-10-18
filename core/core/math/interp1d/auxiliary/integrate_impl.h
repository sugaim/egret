#pragma once

#include <concepts>
#include <ranges>
#include <utility>
#include "core/utils/range_utils/find_interval.h"
#include "interval_at.h"

namespace egret_detail::interp1d_impl {
    enum class relative_position_to_grids {
        left_external,
        internal,
        right_external
    };
    
    template <typename G, typename X, typename Less>
    constexpr auto get_relative_position_enum(const G& front, const G& back, const X& from, const X& to, const Less& less)
        -> std::pair<relative_position_to_grids, relative_position_to_grids>
    {
        using enum relative_position_to_grids;
        return {
            std::invoke(less, from, front) ? left_external :
                std::invoke(less, from, back) ? internal :
                right_external,
            std::invoke(less, to, front) ? left_external :
                std::invoke(less, to, back) ? internal :
                right_external,
        };
    }

    template <
        typename ResultType,
        typename FL, typename FPI, typename FFI, typename FR,
        std::ranges::forward_range Xs, std::ranges::forward_range Ys, typename X, typename Less
    >
        requires 
            std::invocable<const FL&, const X&, const X&> &&
            std::invocable<const FR&, const X&, const X&> &&
            std::invocable<
                const FPI&, 
                std::ranges::iterator_t<const Xs>, std::ranges::iterator_t<const Xs>,
                std::ranges::iterator_t<const Ys>, std::ranges::iterator_t<const Ys>,
                const X&, const X&
            > &&
            std::invocable<
                const FFI&, 
                std::ranges::iterator_t<const Xs>, std::ranges::iterator_t<const Xs>,
                std::ranges::iterator_t<const Ys>, std::ranges::iterator_t<const Ys>
            > &&
            std::strict_weak_order<const Less&, std::ranges::range_reference_t<Xs>, X>
    constexpr auto integrate_impl(
        const FL& left_extrap_integration,
        const FPI& internal_partial_integration,
        const FFI& internal_full_integration,
        const FR& right_extrap_integration,
        const Xs& grids,
        const Ys& values,
        const X& from,
        const X& to,
        const Less& less
    )
    {
        const auto& front = *std::ranges::begin(grids);
        const auto& back = *(std::ranges::next(std::ranges::begin(grids), std::ranges::distance(grids) - 1));
        const auto [from_pos, to_pos] = interp1d_impl::get_relative_position_enum(front, back, from, to, less);
        auto result = static_cast<ResultType>(0);

        // left extrapolation part
        if (from_pos == relative_position_to_grids::left_external) {
            if (to_pos == relative_position_to_grids::left_external) {
                result += left_extrap_integration(from, to);
                return result;
            }
            else {
                result += left_extrap_integration(from, static_cast<X>(front));
            }
        }

        // right extrapolation part
        if (to_pos == relative_position_to_grids::right_external) {
            if (from_pos == relative_position_to_grids::right_external) {
                result += right_extrap_integration(from, to);
                return result;
            }
            else {
                result += right_extrap_integration(static_cast<X>(back), to);
            }
        }

        // internal part. [from_pos, right_pos] is eigher of
        //      [left_external, internal],
        //      [left_external, right_external],
        //      [internal, internal],
        //      [internal, right_external]
        const auto bounded_from = from_pos == relative_position_to_grids::left_external ? static_cast<X>(front) : static_cast<X>(from);
        const auto bounded_to = to_pos == relative_position_to_grids::right_external ?  static_cast<X>(back) : static_cast<X>(to);

        auto [from_xit, from_nxt_xit] = egret::util::find_interval(grids, from, less);
        auto [from_yit, from_nxt_yit] = egret_detail::interp1d_impl::interval_at(
            static_cast<std::size_t>(std::ranges::distance(std::ranges::begin(grids), from_xit)), 
            values
        );
        const auto [to_xit, to_nxt_xit] = egret::util::find_interval(grids, to, less);
        const auto [to_yit, to_nxt_yit] = egret_detail::interp1d_impl::interval_at(
            static_cast<std::size_t>(std::ranges::distance(std::ranges::begin(grids), to_xit)), 
            values
        );

        if (from_xit == to_xit) {
            // from and to belongs to the same interval.
            result += internal_partial_integration(from_xit, from_nxt_xit, from_yit, from_nxt_yit, bounded_from, bounded_to);
            return result;
        }

        // first/last interval
        {
            result += internal_partial_integration(from_xit, from_nxt_xit, from_yit, from_nxt_yit, bounded_from, static_cast<X>(*from_nxt_xit));
            result += internal_partial_integration(to_xit, to_nxt_xit, to_yit, to_nxt_yit, static_cast<X>(*to_xit), bounded_to);
            ++from_xit; ++from_nxt_xit;
            ++from_yit; ++from_nxt_yit;
        }

        if (from_xit == to_xit) {
            return result;
        }
        for (; from_nxt_xit != to_xit; ++from_xit, ++from_nxt_xit, ++from_yit, ++from_nxt_yit) {
            result += internal_full_integration(from_xit, from_nxt_xit, from_yit, from_nxt_yit);
        }
        return result;
    }

} // namespace egret_detail::interp1d_impl