#pragma once

#include "../concepts.h"

namespace egret_detail::interp1d_impl {
// -----------------------------------------------------------------------------
//  [struct] relpos_helper
// -----------------------------------------------------------------------------
    template <typename T>
    struct relpos_helper {
        template <typename U>
            requires egret::math::interp1d::relpos_computable_from<T, U>
        constexpr auto between(const U& from, const U& to) const
            -> egret::math::interp1d::relpos_t<T, U>
        {
            return egret::math::interp1d::distance(from, point.get())
                / egret::math::interp1d::distance(from, to);
        }

        const T* point;        
    };

// -----------------------------------------------------------------------------
//  [fn] relpos
// -----------------------------------------------------------------------------
    template <typename T>
        requires std::is_lvalue_reference_v<T>
    constexpr relpos_helper<std::remove_cvref_t<T>> relpos(T&& p) 
    {
        relpos_helper<T> result;
        result.point = std::addressof(p);
        return result;
    }

} // namespace egret_detail::interp1d_impl
