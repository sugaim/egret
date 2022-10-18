#pragma once

#include <type_traits>
#include <compare>
#include <format>
#include <nlohmann/json_fwd.hpp>
#include "core/utils/string_utils/case_insensitive.h"
#include "core/utils/string_utils/trim.h"
#include "core/utils/string_utils/parse.h"
#include "core/utils/json_utils/j2obj.h"
#include "core/concepts/non_void.h"

namespace egret::mkt {
// -----------------------------------------------------------------------------
//  [class] rate
// -----------------------------------------------------------------------------
    template <typename T>
        requires std::is_constructible_v<T, double>
    class rate {
    private:
        using this_type = rate;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        constexpr rate() noexcept(std::is_nothrow_constructible_v<T, double>) : value_(0.) {}
        constexpr rate(const this_type&) noexcept(std::is_nothrow_copy_constructible_v<T>) = default;
        constexpr rate(this_type&&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;

        constexpr rate(T value)
            noexcept(std::is_nothrow_move_constructible_v<T>)
            : value_(std::move(value))
        {
        }

        static constexpr rate from_value(T value) { return rate(std::move(value)); }
        static constexpr rate from_percent(T percent) { return rate(std::move(percent) / 100.); }
        static constexpr rate from_bps(T bps) { return rate(std::move(bps) / 10000.); }

        constexpr this_type& operator =(const this_type&) noexcept(std::is_nothrow_copy_assignable_v<T>) = default;
        constexpr this_type& operator =(this_type&&) noexcept(std::is_nothrow_move_assignable_v<T>) = default;

    // -------------------------------------------------------------------------
    //  comp assign
    //
        template <typename AT>
            requires requires (T& value, const AT& mult) { value *= mult; }
        constexpr this_type& operator*=(const AT& mult)
        {
            value_ *= mult;
            return *this;
        }
        template <typename AT>
            requires requires (T& value, const AT& mult) { value /= mult; }
        constexpr this_type& operator/=(const AT& mult)
        {
            value_ /= mult;
            return *this;
        }

    // -------------------------------------------------------------------------
    //  parse
    //
        static this_type parse(std::string_view str)
        {
            using sv_t = util::case_insensitive_basic_string_view<char>;
            sv_t sv {str.data(), str.size()};
            sv = util::trim(sv);
            if (0 < sv.size() && sv.back() == '%') {
                sv.remove_suffix(1);
                sv = util::trim(sv);
                return this_type::from_percent(util::from_string<T>(std::string_view(sv.data(), sv.size())));
            }
            if (1 < sv.size() && sv.substr(sv.size() - 2) == "bp") {
                sv.remove_suffix(2);
                sv = util::trim(sv);
                return this_type::from_bps(util::from_string<T>(std::string_view(sv.data(), sv.size())));
            }
            if (2 < sv.size() && sv.substr(sv.size() - 3) == "bps") {
                sv.remove_suffix(3);
                sv = util::trim(sv);
                return this_type::from_bps(util::from_string<T>(std::string_view(sv.data(), sv.size())));
            }
            else {
                return this_type::from_value(util::from_string<T>(std::string_view(sv.data(), sv.size())));
            }
        }

    // -------------------------------------------------------------------------
    //  compare
    //
        constexpr auto operator<=>(const this_type&) const
            requires std::three_way_comparable<T> = default;

    // -------------------------------------------------------------------------
    //  get
    //
        constexpr const T& value() const noexcept { return value_; }
        constexpr T percent() const { return value_ * 100.; }
        constexpr T bps() const { return value_ * 10000.; }

    private:
        T value_;

    }; // class rate

    template <typename T, typename U>
        requires requires (const T& r, const U& mult) { {r * mult} -> cpt::non_void; }
    constexpr auto operator *(const rate<T>& r, const U& mult)
    {
        return rate(r.value() * mult);
    }

    template <typename T, typename U>
        requires requires (const T& r, const U& mult) { {r * mult} -> cpt::non_void; }
    constexpr auto operator *(const U& mult, const rate<T>& r)
    {
        return rate(r.value() * mult);
    }

    template <typename T, typename U>
        requires requires (const T& r, const U& mult) { {r / mult} -> cpt::non_void; }
    constexpr auto operator /(const rate<T>& r, const U& mult)
    {
        return rate(r.value() / mult);
    }

    template <typename T, typename U>
        requires requires (const T& lhs, const U& rhs) { {lhs + rhs} -> cpt::non_void; }
    constexpr auto operator +(const rate<T>& lhs, const rate<U>& rhs)
    {
        return rate(lhs.value() + rhs.value());
    }

    template <typename T, typename U>
        requires requires (const T& lhs, const U& rhs) { {lhs - rhs} -> cpt::non_void; }
    constexpr auto operator -(const rate<T>& lhs, const rate<U>& rhs)
    {
        return rate(lhs.value() - rhs.value());
    }

} // namespace egret::mkt

namespace std {
// -----------------------------------------------------------------------------
//  [struct] formatter
// -----------------------------------------------------------------------------
    template <typename T, typename Char>
    struct formatter<egret::mkt::rate<T>, Char> : formatter<T, Char> {
        template <typename FmtCtxt>
        auto format(const egret::mkt::rate<T>& rate, FmtCtxt& ctxt) const
        {
            auto it = formatter<T, Char>::format(rate.bps(), ctxt);
            *it = 'b'; ++it;
            *it = 'p'; ++it;
            return it;
        }
    };

} // namespace std

namespace nlohmann {
    template <typename R>
    struct adl_serializer<egret::mkt::rate<R>> {
        template <typename Json>
        static egret::mkt::rate<R> from_json(const Json& j)
        {
            if (j.is_number()) {
                return egret::mkt::rate<R>::from_value(static_cast<R>(j.get<double>()));
            }
            else {
                constexpr auto deser = egret::util::j2obj::string.parse_to<egret::mkt::rate<R>>();
                return deser(j);
            }
        }
        template <typename Json>
        static void to_json(Json& j, const egret::mkt::rate<R>& rate)
        {
            j = std::format("{}", rate);
        }
    };

} // namespace nlohmann