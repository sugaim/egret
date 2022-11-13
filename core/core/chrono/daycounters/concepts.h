#pragma once

#include <chrono>
#include <concepts>

namespace egret_detail::dcf_impl {
    void dcf(auto&&, auto&&, auto&&) = delete;

    class dcf_t {
    public:
        template <typename C, typename TimePoint>
            requires
                std::is_invocable_r_v<double, const C&, const TimePoint&, const TimePoint&> ||
                requires (const C& counter, const TimePoint& from, const TimePoint& to) {
                    { dcf(counter, from, to) } -> std::convertible_to<double>;
                }
        constexpr double operator()(const C& counter, const TimePoint& from, const TimePoint& to) const
        {
            if constexpr (std::is_invocable_r_v<double, const C&, const TimePoint&, const TimePoint&>) {
                return std::invoke(counter, from, to);
            }
            else {
                return dcf(counter, from, to);
            }
        }

        template <typename C, typename TimePoint>
            requires
                std::is_invocable_r_v<double, const C&, const TimePoint&, const TimePoint&> &&
                requires (const C& counter, const TimePoint& from, const TimePoint& to) {
                    { dcf(counter, from, to) } -> std::convertible_to<double>;
                }
        constexpr double operator()(const C& counter, const TimePoint& from, const TimePoint& to) const
        {
            if constexpr (std::is_invocable_r_v<double, const C&, const TimePoint&, const TimePoint&>) {
                return std::invoke(counter, from, to);
            }
            else {
                return dcf(counter, from, to);
            }
        }
        
    }; // class dcf_t

} // namespace egret_detail::dcf_impl

namespace egret::chrono::inline cpo {
// -----------------------------------------------------------------------------
//  [cpo] dcf
// -----------------------------------------------------------------------------
    inline constexpr auto dcf = egret_detail::dcf_impl::dcf_t {};
    
} // namespace egret::chrono::inline cpo

namespace egret::cpt {
// -----------------------------------------------------------------------------
//  [concept] daycounter
// -----------------------------------------------------------------------------
    template <typename C, typename TimePoint = std::chrono::sys_days>
    concept daycounter = 
        std::copy_constructible<std::remove_cvref_t<C>> &&
        requires (const C& counter, const TimePoint& from, const TimePoint& to) {
            { chrono::dcf(counter, from, to) } -> std::convertible_to<double>;
        };

} // namespace egret::cpt
