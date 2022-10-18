#pragma once

#include <type_traits>

namespace egret::cpt {
// -----------------------------------------------------------------------------
//  [concept] decayed
// -----------------------------------------------------------------------------
    template <typename T>
    concept decayed = std::is_same_v<T, std::decay_t<T>>;

} // namespace egret::cpt
