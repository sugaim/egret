#pragma once

#include <chrono>
#include <concepts>
#include <vector>
#include <ranges>
#include <nlohmann/json_fwd.hpp>
#include "core/utils/json_utils/j2obj.h"

namespace egret::inst::cfs {
// -----------------------------------------------------------------------------
//  [concept] leg_header
//  [concept] leg_cashflow
// -----------------------------------------------------------------------------
    template <typename T>
    concept leg_header = 
        requires (const T& obj) {
            { obj.discount_curve };
        };

    template <typename T>
    concept leg_cashflow = 
        requires (const T& obj) {
            { obj.payment_date } -> std::convertible_to<std::chrono::sys_days>;
            { obj.entitlement_date } -> std::convertible_to<std::chrono::sys_days>;
            { obj.cashout_date } -> std::convertible_to<std::chrono::sys_days>;
        };

// -----------------------------------------------------------------------------
//  [concept] coupon_leg_header
//  [concept] coupon_leg_cashflow
// -----------------------------------------------------------------------------
    template <typename T>
    concept coupon_leg_header =
        leg_header<T> &&
        requires (const T& obj) {
            { obj.notional };
            { obj.accrual_daycounter };
        };    

    template <typename T>
    concept coupon_leg_cashflow =
        leg_cashflow<T> &&
        requires (const T& obj) {
            { obj.notional_ratio };
            { obj.accrual_start } -> std::convertible_to<std::chrono::sys_days>;
            { obj.accrual_end } -> std::convertible_to<std::chrono::sys_days>;
        };    

// -----------------------------------------------------------------------------
//  [class] leg
// -----------------------------------------------------------------------------
    template <leg_header H, leg_cashflow C>
    class leg {
    private:
        using this_type = leg;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        leg() = delete;
        leg(const this_type&) = default;
        leg(this_type&&) noexcept(std::is_nothrow_move_constructible_v<H>) = default;

        leg(H header, std::vector<C> cashflows)
            noexcept(std::is_nothrow_move_constructible_v<H>)
            : header_(std::move(header)), cashflows_(std::move(cashflows))
        {
            std::ranges::sort(cashflows_, {}, &C::payment_date);
        }

        this_type& operator =(const this_type&) = default;
        this_type& operator =(this_type&&) noexcept(std::is_nothrow_move_assignable_v<H>) = default;

    // -------------------------------------------------------------------------
    //  get
    //
        const H& header() const & noexcept { return header_; }
        H&& header() && noexcept { return std::move(header_); }
        const std::vector<C>& cashflows() const & noexcept { return cashflows_; }
        std::vector<C>&& cashflows() && noexcept { return std::move(cashflows_); }

    private:
        H header_;
        std::vector<C> cashflows_;

    }; // class leg

} // namespace egret::inst::cfs

namespace nlohmann {
    template <typename H, typename C>
    struct adl_serializer<egret::inst::cfs::leg<H, C>> {
        using target_type = egret::inst::cfs::leg<H, C>;

        static constexpr auto deser = egret::util::j2obj::construct<target_type>(
            "header" >> egret::util::j2obj::get<H>,
            "cashflows" >> egret::util::j2obj::array.to_vector_of<C>()
        );

        template <typename Json>
            requires requires (const Json& j) {
                deser(j);
            }
        static target_type from_json(const Json& j)
        {
            return deser(j);
        }

        template <typename Json>
            requires
                std::is_assignable_v<Json&, const H&> &&
                std::is_assignable_v<Json&, const C&>
        static void to_json(Json& j, const target_type& obj)
        {
            j["header"] = obj.header();
            j["cashflows"] = obj.cashflows();
        }
    };

} // namespace nlohmann