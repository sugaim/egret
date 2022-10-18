#pragma once

#include "core/auto_link.h"
#include <array>
#include <cstdint>
#include <chrono>
#include <string>
#include <string_view>
#include <format>

namespace egret::chrono {
// -----------------------------------------------------------------------------
//  [enum] tenor_unit
// -----------------------------------------------------------------------------
    enum class tenor_unit {
        days,
        weeks,
        months,
        years
    };

// -----------------------------------------------------------------------------
//  [class] tenor
// -----------------------------------------------------------------------------
    class tenor {
    private:
        using this_type = tenor;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        constexpr tenor() noexcept = default;
        constexpr tenor(const this_type&) noexcept = default;
        constexpr tenor(this_type&&) noexcept = default;

        constexpr tenor(std::int32_t count, tenor_unit unit) noexcept : count_(count), unit_(unit) {}
        constexpr tenor(const std::chrono::days& days) noexcept : tenor(days.count(), tenor_unit::days) {}
        constexpr tenor(const std::chrono::weeks& weeks) noexcept : tenor(weeks.count(), tenor_unit::weeks) {}
        constexpr tenor(const std::chrono::months& months) noexcept : tenor(months.count(), tenor_unit::months) {}
        constexpr tenor(const std::chrono::years& years) noexcept : tenor(years.count(), tenor_unit::years) {}

        static constexpr tenor days(std::int32_t count) noexcept { return tenor(count, tenor_unit::days); }
        static constexpr tenor weeks(std::int32_t count) noexcept { return tenor(count, tenor_unit::weeks); }
        static constexpr tenor months(std::int32_t count) noexcept { return tenor(count, tenor_unit::months); }
        static constexpr tenor years(std::int32_t count) noexcept { return tenor(count, tenor_unit::years); }

        constexpr this_type& operator =(const this_type&) noexcept = default;
        constexpr this_type& operator =(this_type&&) noexcept = default;
        constexpr this_type& operator =(const std::chrono::days& days) noexcept { return *this = tenor(days); }
        constexpr this_type& operator =(const std::chrono::weeks& weeks) noexcept { return *this = tenor(weeks); }
        constexpr this_type& operator =(const std::chrono::months& months) noexcept { return *this = tenor(months); }
        constexpr this_type& operator =(const std::chrono::years& years) noexcept { return *this = tenor(years); }

    // -------------------------------------------------------------------------
    //  arithmetics
    //
        constexpr this_type operator +() const noexcept { return *this; }
        constexpr this_type operator -() const noexcept { return tenor(-count_, unit_); }

        constexpr this_type& operator*=(std::int32_t mult) noexcept { count_ *= mult; return *this; }
        friend constexpr this_type operator *(this_type self, std::int32_t mult) noexcept { self *= mult; return self; }
        friend constexpr this_type operator *(std::int32_t mult, this_type self) noexcept { self *= mult; return self; }

        constexpr this_type& operator/=(std::int32_t mult) noexcept { count_ /= mult; return *this; }
        friend constexpr this_type operator /(this_type self, std::int32_t mult) noexcept { self /= mult; return self; }

        constexpr this_type& operator%=(std::int32_t mult) noexcept { count_ %= mult; return *this; }
        friend constexpr this_type operator %(this_type self, std::int32_t mult) noexcept { self %= mult; return self; }

        // hidden friends
        friend std::chrono::sys_days operator +(const std::chrono::sys_days& lhs, const tenor& tnr);
        friend std::chrono::sys_days operator -(const std::chrono::sys_days& lhs, const tenor& tnr);
        friend std::chrono::sys_seconds operator +(const std::chrono::sys_seconds& lhs, const tenor& tnr);
        friend std::chrono::sys_seconds operator -(const std::chrono::sys_seconds& lhs, const tenor& tnr);

    // -------------------------------------------------------------------------
    //  parse
    //
        static tenor parse(std::string_view sv);

    // -------------------------------------------------------------------------
    //  get 
    //
        constexpr std::int32_t count() const noexcept { return count_; }
        constexpr tenor_unit unit() const noexcept { return unit_; }

    private:
        std::int32_t count_ = 0;
        tenor_unit unit_ = tenor_unit::days;

    }; // class tenor

} // namespace egret::chrono

namespace egret::tenor_literals {
    constexpr chrono::tenor operator"" _days(unsigned long long count) noexcept
    {
        return chrono::tenor::days(static_cast<std::int32_t>(count));
    }
    constexpr chrono::tenor operator"" _weeks(unsigned long long count) noexcept
    {
        return chrono::tenor::weeks(static_cast<std::int32_t>(count));
    }
    constexpr chrono::tenor operator"" _months(unsigned long long count) noexcept
    {
        return chrono::tenor::months(static_cast<std::int32_t>(count));
    }
    constexpr chrono::tenor operator"" _years(unsigned long long count) noexcept
    {
        return chrono::tenor::years(static_cast<std::int32_t>(count));
    }

} // namespace egret::chrono::tenor_literals

namespace std {
    template <>
    struct formatter<egret::chrono::tenor> : formatter<string_view> {
        template <typename FmtCtx>
        constexpr auto format(const egret::chrono::tenor& tenor, FmtCtx& ctx) const
        {
            std::array<char, 24> chars = {'\0'};
            const auto [end, err] = std::to_chars(chars.data(), chars.data() + chars.size(), tenor.count());
            if (err != std::errc()) {
                throw format_error("Fail to convert tenor count into chars");
            }
            switch (tenor.unit()) {
            case egret::chrono::tenor_unit::days:   *end = 'D';
            case egret::chrono::tenor_unit::weeks:  *end = 'W';
            case egret::chrono::tenor_unit::months: *end = 'M';
            case egret::chrono::tenor_unit::years:  *end = 'Y';
            }
            std::string_view str (chars.data(), end - chars.data() + 1);
            return formatter<string_view>::format(str, ctx);
        }
    };

} // namespace std