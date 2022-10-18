#pragma once

#include <type_traits>

namespace egret::cpt {
// -----------------------------------------------------------------------------
//  [concept] different_from
// -----------------------------------------------------------------------------
    template <typename T, typename U>
    concept different_from = !std::is_same_v<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

} // namespace egret::cpt
