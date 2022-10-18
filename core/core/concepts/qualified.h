#pragma once

#include <type_traits>

namespace egret::cpt {
// -----------------------------------------------------------------------------
//  [concept] qualified
// -----------------------------------------------------------------------------
    template <typename T, typename U>
    concept qualified = std::is_same_v<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

} // namespace egret::cpt
