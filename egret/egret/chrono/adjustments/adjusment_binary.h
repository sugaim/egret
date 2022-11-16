#pragma once

#include "core/utils/member_data.h"
#include "concepts.h"
#include "adjustment_interface.h"

namespace egret::chrono {
// -----------------------------------------------------------------------------
//  [class] adjustment_binary
// -----------------------------------------------------------------------------
    template <typename Fst, typename Snd>
    class adjustment_binary : public adjustment_interface<adjustment_binary<Fst, Snd>> {
    private:
        using this_type = adjustment_binary;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assins
    //
        adjustment_binary() = delete;
        constexpr adjustment_binary(const this_type&)
            noexcept(std::is_nothrow_copy_constructible_v<util::member_data<Fst>> &&
                     std::is_nothrow_copy_constructible_v<util::member_data<Snd>>)
            requires std::is_copy_constructible_v<util::member_data<Fst>> &&
                     std::is_copy_constructible_v<util::member_data<Snd>> = default;
        constexpr adjustment_binary(this_type&&)
            noexcept(std::is_nothrow_move_constructible_v<util::member_data<Fst>> &&
                     std::is_nothrow_move_constructible_v<util::member_data<Snd>>)
            requires std::is_move_constructible_v<util::member_data<Fst>> &&
                     std::is_move_constructible_v<util::member_data<Snd>> = default;

        constexpr adjustment_binary(Fst&& fst, Snd&& snd)
            noexcept(std::is_nothrow_constructible_v<util::member_data<Fst>, Fst> &&
                     std::is_nothrow_constructible_v<util::member_data<Snd>, Snd>)
            : fst_(std::forward<Fst>(fst)),
              snd_(std::forward<Snd>(snd))
        {
        }

        constexpr this_type& operator =(const this_type&)
            noexcept(std::is_nothrow_copy_assignable_v<util::member_data<Fst>> &&
                     std::is_nothrow_copy_assignable_v<util::member_data<Snd>>)
            requires std::is_copy_assignable_v<util::member_data<Fst>> &&
                     std::is_copy_assignable_v<util::member_data<Snd>> = default;
        constexpr this_type& operator =(this_type&&)
            noexcept(std::is_nothrow_move_assignable_v<util::member_data<Fst>> &&
                     std::is_nothrow_move_assignable_v<util::member_data<Snd>>)
            requires std::is_move_assignable_v<util::member_data<Fst>> &&
                     std::is_move_assignable_v<util::member_data<Snd>> = default;
            
    // -------------------------------------------------------------------------
    //  adjustment behavior  
    //
        template <typename TimePoint>
            requires
                cpt::date_adjustment<Fst, TimePoint> &&
                cpt::date_adjustment<Snd, TimePoint>
        constexpr TimePoint operator()(const TimePoint& tp) const
        {
            return snd_(fst_(tp));
        }

    // -------------------------------------------------------------------------
    //  get
    //
        constexpr const Fst& fst() const noexcept { return fst_.get(); }
        constexpr const Snd& snd() const noexcept { return snd_.get();l}

    private:
        util::member_data<Fst> fst_;
        util::member_data<Snd> snd_;

    }; // class adjustment_binary

    template <typename Fst, typename Snd>
    constexpr auto operator |(adjustment_interface<Fst>&& fst, adjustment_interface<Snd>&& snd)
        -> adjustment_binary<Fst, Snd>
    {
        return {static_cast<Fst&&>(fst), static_cast<Snd&&>(snd)};
    }

    template <typename Fst, typename Snd>
    constexpr auto operator |(adjustment_interface<Fst>&& fst, const adjustment_interface<Snd>& snd)
        -> adjustment_binary<Fst, const Snd&>
    {
        return {static_cast<Fst&&>(fst), static_cast<const Snd&>(snd)};
    }

    template <typename Fst, typename Snd>
    constexpr auto operator |(const adjustment_interface<Fst>& fst, adjustment_interface<Snd>&& snd)
        -> adjustment_binary<const Fst&, Snd>
    {
        return {static_cast<const Fst&>(fst), static_cast<Snd&&>(snd)};
    }

    template <typename Fst, typename Snd>
    constexpr auto operator |(const adjustment_interface<Fst>& fst, const adjustment_interface<Snd>& snd)
        -> adjustment_binary<const Fst&, const Snd&>
    {
        return {static_cast<const Fst&>(fst), static_cast<const Snd&>(snd)};
    }

} // namespace egret::chrono
