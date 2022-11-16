#pragma once

#include <chrono>
#include "core/chrono/calendars/calendar.h"
#include "adjustment_interface.h"

namespace egret::chrono {
// -----------------------------------------------------------------------------
//  [enum] holiday_adjustment_type
// -----------------------------------------------------------------------------
    enum class holiday_adjustment_type : char {
        unadjust,
        following,
        preceeding,
        modified_following,
        modified_preceeding,
    };

    constexpr auto string_value_map_of(std::type_identity<holiday_adjustment_type>)
    {
        using pair_t = std::pair<const char*, holiday_adjustment_type>;
        return std::array {
            pair_t {"unadjust", holiday_adjustment_type::unadjust},
            pair_t {"U", holiday_adjustment_type::unadjust},
            pair_t {"following", holiday_adjustment_type::following},
            pair_t {"F", holiday_adjustment_type::following},
            pair_t {"modified_following", holiday_adjustment_type::modified_following},
            pair_t {"MF", holiday_adjustment_type::modified_following},
            pair_t {"preceeding", holiday_adjustment_type::preceeding},
            pair_t {"P", holiday_adjustment_type::preceeding},
            pair_t {"modified_preceeding", holiday_adjustment_type::modified_preceeding},
            pair_t {"MP", holiday_adjustment_type::modified_preceeding},
        };
    }
    
} // namespace egret::chrono

namespace egret_detail::hadj_impl {
// -----------------------------------------------------------------------------
//  [struct] type_constant
// -----------------------------------------------------------------------------
    template <egret::chrono::holiday_adjustment_type type>
    using type_constant = std::integral_constant<egret::chrono::holiday_adjustment_type, type>;

    template <egret::chrono::holiday_adjustment_type type>
    std::chrono::sys_days adjust(const std::chrono::sys_days& d, const egret::chrono::calendar& cal, type_constant<type>);

    template <egret::chrono::holiday_adjustment_type type>
    std::chrono::sys_seconds adjust(const std::chrono::sys_seconds& dt, const egret::chrono::calendar& cal, type_constant<type>)
    {
        const auto d = std::chrono::time_point_cast<std::chrono::days>(dt);
        return hadj_impl::adjust(d, cal, type_constant<type>{}) + (dt - d);
    }

    std::chrono::sys_days adjust(const std::chrono::sys_days& d, const egret::chrono::calendar&, type_constant<egret::chrono::holiday_adjustment_type::unadjust>)
    {
        return d;
    }
    std::chrono::sys_seconds adjust(const std::chrono::sys_seconds& dt, const egret::chrono::calendar&, type_constant<egret::chrono::holiday_adjustment_type::unadjust>)
    {
        return dt;
    }

    std::chrono::sys_days adjust(const std::chrono::sys_days& d, const egret::chrono::calendar& cal, egret::chrono::holiday_adjustment_type type);
    std::chrono::sys_seconds adjust(const std::chrono::sys_seconds& d, const egret::chrono::calendar& cal, egret::chrono::holiday_adjustment_type type);

// -----------------------------------------------------------------------------
//  [class] holiday_adjustment_impl
// -----------------------------------------------------------------------------
    template <typename Tag>
    class holiday_adjustment_impl {
    private:
        using this_type = holiday_adjustment_impl;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns  
    //
        holiday_adjustment_impl() = delete;
        holiday_adjustment_impl(const this_type&) noexcept = default;
        holiday_adjustment_impl(this_type&&) noexcept = default;

        holiday_adjustment_impl(egret::chrono::calendar&& cal, Tag tag) noexcept : cal_(std::move(cal)), tag_(tag) {}
        holiday_adjustment_impl(const egret::chrono::calendar& cal, Tag tag) noexcept : cal_(cal), tag_(tag) {}

        explicit holiday_adjustment_impl(egret::chrono::calendar&& cal, Tag tag) noexcept
            requires std::is_empty_v<Tag> 
            : holiday_adjustment_impl(std::move(cal), Tag{}) 
        {
        }

        explicit holiday_adjustment_impl(const egret::chrono::calendar& cal) noexcept
            requires std::is_empty_v<Tag> 
            : holiday_adjustment_impl(cal, Tag{}) 
        {
        }

        this_type& operator =(const this_type&) noexcept = default;
        this_type& operator =(this_type&&) noexcept = default;

    // -------------------------------------------------------------------------
    //  adjustment behavior
    //
        auto operator()(const std::chrono::sys_days& d) const -> std::chrono::sys_days { return hadj_impl::adjust(d, cal_, tag_); }
        auto operator()(const std::chrono::sys_seconds& dt) const -> std::chrono::sys_seconds { return hadj_impl::adjust(dt, cal_, tag_); }

    // -------------------------------------------------------------------------
    //  get
    //
        const egret::chrono::calendar& calendar() const noexcept { return cal_; }
        egret::chrono::holiday_adjustment_type type() const noexcept
        { 
            if constexpr (std::is_empty_v<Tag>) {
                return Tag::value;
            }
            else {
                return tag_; 
            }
        }

    private:
        egret::chrono::calendar cal_;
        [[no_unique_address]] Tag tag_;

    }; // class holiday_adjustment_impl

} // namespace egret_detail::hadj_impl

namespace egret::chrono {
// -----------------------------------------------------------------------------
//  [fn] adjust
// -----------------------------------------------------------------------------
    template <holiday_adjustment_type type>
    auto adjust(const std::chrono::sys_days& d, const calendar& cal) -> std::chrono::sys_days
    {
        constexpr auto tag = egret_detail::hadj_impl::type_constant<type> {};
        return egret_detail::hadj_impl::adjust(d, cal, tag);
    }

    template <holiday_adjustment_type type>
    auto adjust(const std::chrono::sys_seconds& dt, const calendar& cal) -> std::chrono::sys_seconds
    {
        const auto d = std::chrono::time_point_cast<std::chrono::days>(dt);
        return chrono::adjust<type>(d, cal) + (dt - d);
    }

    auto adjust(const std::chrono::sys_days& d, const calendar& cal, holiday_adjustment_type type) -> std::chrono::sys_days;
    auto adjust(const std::chrono::sys_seconds& dt, const calendar& cal, holiday_adjustment_type type) -> std::chrono::sys_seconds;

// -----------------------------------------------------------------------------
//  [class] holiday_adjustment
// -----------------------------------------------------------------------------
    class holiday_adjustment : 
        public egret_detail::hadj_impl::holiday_adjustment_impl<holiday_adjustment_type>,
        public adjustment_interface<holiday_adjustment> {
    private:
        using super_type = egret_detail::hadj_impl::holiday_adjustment_impl<holiday_adjustment_type>;

        class unadjust_t : public adjustment_interface<unadjust_t> {
        public:
            constexpr auto operator()(const std::chrono::sys_days& d) const { return d; }
            constexpr auto operator()(const std::chrono::sys_seconds& dt) const { return dt; }

            holiday_adjustment_type type() const noexcept { return holiday_adjustment_type::unadjust; }

        }; // class unadjust_t

        class following_t : public egret_detail::hadj_impl::holiday_adjustment_impl<
            egret_detail::hadj_impl::type_constant<holiday_adjustment_type::following>>,
            public adjustment_interface<following_t> {
        private:
            using super_type = egret_detail::hadj_impl::holiday_adjustment_impl<
                egret_detail::hadj_impl::type_constant<holiday_adjustment_type::following>>;
        public:
            using super_type::super_type;
            using super_type::operator =;
            using super_type::operator();
            using super_type::calendar;
            using super_type::type;

        }; // class following_t

        class preceeding_t : 
            public egret_detail::hadj_impl::holiday_adjustment_impl<
                egret_detail::hadj_impl::type_constant<holiday_adjustment_type::preceeding>>,
            public adjustment_interface<following_t> {
        private:
            using super_type = egret_detail::hadj_impl::holiday_adjustment_impl<
                egret_detail::hadj_impl::type_constant<holiday_adjustment_type::preceeding>>;
        public:
            using super_type::super_type;
            using super_type::operator =;
            using super_type::operator();
            using super_type::calendar;
            using super_type::type;

        }; // class preceeding_t
    
        class modified_following_t : 
            public egret_detail::hadj_impl::holiday_adjustment_impl<
                egret_detail::hadj_impl::type_constant<holiday_adjustment_type::modified_following>>,
            public adjustment_interface<following_t> {
        private:
            using super_type = egret_detail::hadj_impl::holiday_adjustment_impl<
                egret_detail::hadj_impl::type_constant<holiday_adjustment_type::modified_following>>;
        public:
            using super_type::super_type;
            using super_type::operator =;
            using super_type::operator();
            using super_type::calendar;
            using super_type::type;

        }; // class modified_following_t

        class modified_preceeding_t : 
            public egret_detail::hadj_impl::holiday_adjustment_impl<
                egret_detail::hadj_impl::type_constant<holiday_adjustment_type::modified_preceeding>>,
            public adjustment_interface<modified_preceeding_t> {
        private:
            using super_type = egret_detail::hadj_impl::holiday_adjustment_impl<
                egret_detail::hadj_impl::type_constant<holiday_adjustment_type::modified_preceeding>>;

        public:
            using super_type::super_type;
            using super_type::operator =;
            using super_type::operator();
            using super_type::calendar;
            using super_type::type;

        }; // class modified_preceeding_t

    public:
        using super_type::super_type;
        using super_type::operator =;
        using super_type::operator();
        using super_type::calendar;
        using super_type::type;

        holiday_adjustment(unadjust_t) noexcept: super_type(chrono::calendar{}, holiday_adjustment_type::unadjust) {}
        holiday_adjustment(const following_t& internal) noexcept: super_type(internal.calendar(), holiday_adjustment_type::following) {}
        holiday_adjustment(const preceeding_t& internal) noexcept: super_type(internal.calendar(), holiday_adjustment_type::preceeding) {}
        holiday_adjustment(const modified_following_t& internal) noexcept: super_type(internal.calendar(), holiday_adjustment_type::modified_following) {}
        holiday_adjustment(const modified_preceeding_t& internal) noexcept: super_type(internal.calendar(), holiday_adjustment_type::modified_preceeding) {}

        static constexpr auto unadjust = unadjust_t {};

        static following_t following(const chrono::calendar& cal) { return following_t {cal}; }
        static following_t following(chrono::calendar&& cal) { return following_t {std::move(cal)}; }

        static preceeding_t preceeding(const chrono::calendar& cal) { return preceeding_t {cal}; }
        static preceeding_t preceeding(chrono::calendar&& cal) { return preceeding_t {std::move(cal)}; }

        static modified_following_t modified_following(const chrono::calendar& cal) { return modified_following_t {cal}; }
        static modified_following_t modified_following(chrono::calendar&& cal) { return modified_following_t {std::move(cal)}; }

        static modified_preceeding_t modified_preceeding(const chrono::calendar& cal) { return modified_preceeding_t {cal}; }
        static modified_preceeding_t modified_preceeding(chrono::calendar&& cal) { return modified_preceeding_t {std::move(cal)}; }

    }; // class holiday_adjustment
    
} // namespace egret::chrono
