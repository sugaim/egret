#pragma once

#include <type_traits>

namespace egret::cpt {
// -----------------------------------------------------------------------------
//  [concept] non_void
// -----------------------------------------------------------------------------
    template <typename T>
    concept non_void = !std::is_void_v<T>;

} // namespace egret::cpt
