#pragma once

#include <concepts>
#include <iterator>
#include <ranges>

namespace egret::math::interp1d {
// -----------------------------------------------------------------------------
//  [concept] slope_generator
// -----------------------------------------------------------------------------
    template <typename T, typename XIt, typename XSn, typename YIt, typename OIt>
    concept slope_generator = 
        std::forward_iterator<XIt> &&
        std::sentinel_for<XSn, XIt> &&
        std::forward_iterator<YIt> &&
        std::output_iterator<OIt, std::iter_value_t<YIt>> &&
        requires (const T& calc, XIt xit, const XSn& xsn, YIt yit, OIt oit) {
            {calc.generate(xit, xsn, yit, oit)} -> std::convertible_to<OIt>;
        };

} // namespace egret::math::interp1d
