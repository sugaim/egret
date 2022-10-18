#pragma once

#include <vector>

namespace egret::util {
// -----------------------------------------------------------------------------
//  [fn] vector_assign
// -----------------------------------------------------------------------------
    template <typename T, typename Rng>
    void vector_assign(std::vector<T>& target, Rng&& rng)
    {
        if constexpr (std::is_same_v<std::vector<T>, Rng>) {
            target = std::move(rng);
            return;
        }

        target.clear();
        if constexpr (std::ranges::sized_range<Rng>) {
            target.reserve(std::ranges::size(rng));
        }
        for (const auto& elem : rng) {
            target.push_back(elem);
        }
    }
    
// -----------------------------------------------------------------------------
//  [fn] strongly_exception_safe_vector_assign
// -----------------------------------------------------------------------------
    template <typename T, typename Rng>
    void strongly_exception_safe_vector_assign(std::vector<T>& target, Rng&& rng)
    {
        std::vector<T> tmp {};
        if constexpr (std::is_same_v<std::vector<T>, Rng>) {
            tmp = std::move(rng);
        }
        else {
            if constexpr (std::ranges::sized_range<Rng>) {
                tmp.reserve(std::ranges::size(rng));
            }
            for (const auto& elem : rng) {
                tmp.push_back(elem);
            }
        }
        target.swap(tmp);
    }
    
} // namespace egret::util
