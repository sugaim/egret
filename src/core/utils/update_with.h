#pragma once

#include <utility>

namespace egret_detail::init_impl {
    void update(auto&, auto&& ...) = delete;

    class update_with_t {
    public:
        template <typename T, typename ...Args>
            requires
                requires (T& obj, Args&& ...args) { obj.update(std::forward<Args>(args)...); } ||
                requires (T& obj, Args&& ...args) { update(obj, std::forward<Args>(args)...); }
        constexpr void operator()(T& obj, Args&& ...args) const
        {
            if constexpr (requires (T& obj, Args&& ...args) { obj.update(std::forward<Args>(args)...); }) {
                return obj.update(std::forward<Args>(args)...);
            }
            else {
                return update(obj, std::forward<Args>(args)...);
            }
        }

    }; // class update_with_t

} // namespace egret_detail::init_impl

namespace egret::util::inline cpo {
// -----------------------------------------------------------------------------
//  [cpo] update_with
// -----------------------------------------------------------------------------
    inline constexpr auto update_with = egret_detail::init_impl::update_with_t {};

} // namespace egret::util::inline cpo

namespace egret::cpt {
// -----------------------------------------------------------------------------
//  [concept] updatable_with
// -----------------------------------------------------------------------------
    template <typename T, typename ...Args>
    concept updatable_with = requires (T & obj, Args&& ...args) {
        util::update_with(obj, std::forward<Args>(args)...);
    };

} // namespace egret::cpt
