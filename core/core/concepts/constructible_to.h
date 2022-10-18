#pragma once

#include <concepts>

namespace egret::cpt {
// -----------------------------------------------------------------------------
//  [concept] constructible_to
// -----------------------------------------------------------------------------
    template <typename T, typename Target>
    concept constructible_to = std::constructible_from<Target, T>;

} // namespace egret::cpt
