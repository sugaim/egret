#pragma once

#include <utility>

namespace egret_detail::init_impl {
    void initialize(auto&, auto&& ...) = delete;

    class initialize_with_t {
    public:
        template <typename T, typename ...Args>
            requires
                requires (T& obj, Args&& ...args) { obj.initialize(std::forward<Args>(args)...); } ||
                requires (T& obj, Args&& ...args) { initialize(obj, std::forward<Args>(args)...); }
        constexpr void operator()(T& obj, Args&& ...args) const
        {
            if constexpr (requires (T& obj, Args&& ...args) { obj.initialize(std::forward<Args>(args)...); }) {
                return obj.initialize(std::forward<Args>(args)...);
            }
            else {
                return initialize(obj, std::forward<Args>(args)...);
            }
        }

    }; // class initialize_with_t

} // namespace egret_detail::init_impl

namespace egret::util::inline cpo {
// -----------------------------------------------------------------------------
//  [cpo] initialize_with
// -----------------------------------------------------------------------------
    inline constexpr auto initialize_with = egret_detail::init_impl::initialize_with_t {};

} // namespace egret::util::inline cpo

namespace egret::cpt {
// -----------------------------------------------------------------------------
//  [concept] initializable_with
// -----------------------------------------------------------------------------
    template <typename T, typename ...Args>
    concept initializable_with = requires (T& obj, Args&& ...args) {
        util::initialize_with(obj, std::forward<Args>(args)...);
    };

} // namespace egret::cpt
