#pragma once

#include <variant>
#include <tuple>
#include <nlohmann/json_fwd.hpp>
#include "concepts.h"

namespace egret::mkt {
// -----------------------------------------------------------------------------
//  [class] daycounter_variant
// -----------------------------------------------------------------------------
    template <typename TimePoint, typename ...DayCounters>
    class daycounter_variant : public std::variant<DayCounters...> {
    private:
        using this_type = daycounter_variant;
        using super_type = std::variant<DayCounters...>;
        static_assert((cpt::daycounter<DayCounters, TimePoint> && ...), "Some DayCounters does not satisfy daycounter concept.");

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtors and assigns
    //
        using super_type::super_type;
        using super_type::operator =;

    private:
        friend class nlohmann::basic_json<>;
        daycounter_variant() = delete;

    public:
        daycounter_variant(const this_type&) 
            noexcept(std::conjunction_v<std::is_nothrow_copy_constructible<DayCounters>...>)
            requires std::conjunction_v<std::is_copy_constructible<DayCounters>...> = default;
        daycounter_variant(this_type&&)
            noexcept(std::conjunction_v<std::is_nothrow_move_constructible<DayCounters>...>)
            requires std::conjunction_v<std::is_move_constructible<DayCounters>...> = default;

    // -------------------------------------------------------------------------
    //  daycounter behavior
    //
        constexpr double operator()(const TimePoint& from, const TimePoint& to) const
        {
            return std::visit(
                [&from, &to](const auto& counter) -> double { return mkt::dcf(counter, from, to); },
                static_cast<const super_type&>(*this)
            );
        }

    }; // class daycounter_variant

    template <typename ...TimePoints, typename ...DayCounters>
    class daycounter_variant<std::tuple<TimePoints...>, DayCounters...> : public std::variant<DayCounters...> {
    private:
        using this_type = daycounter_variant;
        using super_type = std::variant<DayCounters...>;

        template <typename DayCounter>
        static constexpr bool check_each = (cpt::daycounter<DayCounter, TimePoints> && ...);
        static_assert((check_each<DayCounters> && ...), "Some DayCounters does not satisfy daycounter concept.");

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtors and assigns
    //
        using super_type::super_type;
        using super_type::operator =;

        daycounter_variant() = delete;
        daycounter_variant(const this_type&) 
            noexcept(std::conjunction_v<std::is_nothrow_copy_constructible<DayCounters>...>)
            requires std::conjunction_v<std::is_copy_constructible<DayCounters>...> = default;
        daycounter_variant(this_type&&)
            noexcept(std::conjunction_v<std::is_nothrow_move_constructible<DayCounters>...>)
            requires std::conjunction_v<std::is_move_constructible<DayCounters>...> = default;

    // -------------------------------------------------------------------------
    //  daycounter behavior
    // 
        template <typename TimePoint>
            requires std::disjunction_v<std::is_same<TimePoint, TimePoints>...>
        constexpr double operator()(const TimePoint& from, const TimePoint& to) const
        {
            return std::visit(
                [&from, &to](const auto& counter) -> double { 
                    return mkt::dcf(counter, from, to); 
                },
                static_cast<const super_type&>(*this)
            );
        }

    }; // class daycounter_variant

} // namespace egret::mkt
