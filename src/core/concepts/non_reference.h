#pragma once

#include <type_traits>

namespace egret::cpt {
// -----------------------------------------------------------------------------
//  [concept] non_reference
// -----------------------------------------------------------------------------
    template <typename T>
    concept non_reference = !std::is_reference_v<T>;

} // namespace egret::cpt
