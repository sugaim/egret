#pragma once

#include <ranges>
#include <nlohmann/json_fwd.hpp>
#include "core/assertions/assertion.h"
#include "core/utils/json_utils/concepts.h"
#include "core/utils/json_utils/j2obj.h"

namespace egret_detail::interp1d_impl {
// -----------------------------------------------------------------------------
//  [fn] record_knots
// -----------------------------------------------------------------------------
    template <egret::cpt::basic_json Json, std::ranges::forward_range Xs, std::ranges::forward_range Ys>
        requires
            std::is_assignable_v<Json&, std::ranges::range_reference_t<const Xs&>> &&
            std::is_assignable_v<Json&, std::ranges::range_reference_t<const Ys&>>
    void records_knots(Json& j, const Xs& grids, const Ys& values)
    {
        const auto grid_size = static_cast<std::size_t>(std::ranges::distance(grids));
        const auto values_size = static_cast<std::size_t>(std::ranges::distance(values));
        egret::debug_assert(
            grid_size == values_size,
            "Size of grids and values must be the same because they are interpolatee. "
            "[grids.size={}, values.size={}]",
            grid_size, values_size
        );

        std::vector<Json> knots;
        knots.resize(grid_size);
            
        auto xit = std::ranges::begin(grids);
        auto yit = std::ranges::begin(values);

        for (const auto& i : std::ranges::iota_view(0u, grid_size)) {
            knots[i] = {{"grid", *xit}, {"value", *yit}};
            ++xit; ++yit;
        }
        j["knots"] = std::move(knots);
    }

// -----------------------------------------------------------------------------
//  [fn] get_knots_size
// -----------------------------------------------------------------------------
    template <egret::cpt::basic_json Json>
    std::size_t get_knots_size(const Json& j)
    {
        return static_cast<std::size_t>(std::ranges::distance(("knots" >> egret::util::j2obj::array)(j)));
    }

// -----------------------------------------------------------------------------
//  [fn] recover_knots
// -----------------------------------------------------------------------------
    template <typename X, typename Y, egret::cpt::basic_json Json, std::output_iterator<X> XIt, std::output_iterator<Y> YIt>
        requires
            egret::cpt::deserializable_json_with<Json, egret::util::j2obj::get_t<X>> &&
            egret::cpt::deserializable_json_with<Json, egret::util::j2obj::get_t<Y>>
    void recover_knots(const Json& j, XIt xit, YIt yit)
    {
        constexpr auto j2x = "grid" >> egret::util::j2obj::get<X>;
        constexpr auto j2y = "value" >> egret::util::j2obj::get<Y>;
        for (const auto& knot : ("knots" >> egret::util::j2obj::array)(j)) {
            *xit = j2x(knot);
            *yit = j2y(knot);
            ++xit; ++yit;
        }
    }

    template <typename X, typename Y, egret::cpt::basic_json Json, std::output_iterator<std::pair<X, Y>> XYIt>
        requires
            egret::cpt::deserializable_json_with<Json, egret::util::j2obj::get_t<X>> &&
            egret::cpt::deserializable_json_with<Json, egret::util::j2obj::get_t<Y>>
    void recover_knots(const Json& j, XYIt xyit)
    {
        constexpr auto j2x = "grid" >> egret::util::j2obj::get<X>;
        constexpr auto j2y = "value" >> egret::util::j2obj::get<Y>;
        for (const auto& knot : ("knots" >> egret::util::j2obj::array)(j)) {
            *xyit = std::pair(j2x(knot), j2y(knot));
            ++xyit;
        }
    }

    template <typename X, typename Y, egret::cpt::basic_json Json>
        requires
            egret::cpt::deserializable_json_with<Json, egret::util::j2obj::get_t<X>> &&
            egret::cpt::deserializable_json_with<Json, egret::util::j2obj::get_t<Y>>
    auto recover_knots(const Json& j)
        -> std::pair<std::vector<X>, std::vector<Y>>
    {
        const auto sz = interp1d_impl::get_knots_size(j);
        std::vector<X> xs;
        std::vector<Y> ys;
        xs.reserve(sz);
        ys.reserve(sz);

        if constexpr (std::is_swappable_v<std::pair<X, Y>>) {
            std::vector<std::pair<X, Y>> pairs;
            pairs.reserve(sz);

            interp1d_impl::recover_knots<X, Y>(j, std::back_inserter(pairs));
            const auto less = [cmp=std::less<>{}] (const auto& l, const auto& r) { return cmp(l.first, r.first); };
            std::ranges::sort(pairs, less);
            for (auto& [x, y] : pairs) {
                xs.push_back(std::move(x));
                ys.push_back(std::move(y));
            }
        }
        else {
            interp1d_impl::recover_knots<X, Y>(j, std::back_inserter(xs), std::back_inserter(ys));
        }
        return {std::move(xs), std::move(ys)};
    }
    
} // namespace egret_detail::interp1d_impl
