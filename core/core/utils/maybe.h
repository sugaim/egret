#pragma once

#include <optional>
#include <ranges>
#include <compare>
#include "core/concepts/non_reference.h"

namespace egret_detail::maybe_iter_impl {
    template <typename T>
    struct maybe_iterator {
        using value_type = T;
        using iterator_concept = std::forward_iterator_tag;

        constexpr T& operator *() const { return maybe_data.value(); }
        constexpr maybe_iterator& operator++() noexcept { maybe_data.reset(); return *this; }
        constexpr maybe_iterator operator++(int) noexcept { auto self = *this; maybe_data.reset(); return self; }
        constexpr bool operator==(const maybe_iterator& other) const noexcept { return maybe_data.has_value() == other.maybe_data.has_value(); }

        std::optional<std::reference_wrapper<T>> maybe_data;
    };

} // namespace egret_detail::maybe_iter_impl

namespace egret::util {
// -----------------------------------------------------------------------------
//  [class] maybe
// -----------------------------------------------------------------------------
    template <typename T>
        requires cpt::non_reference<T> || std::is_const_v<std::remove_reference_t<T>>
    class maybe : private std::optional<T>, public std::ranges::view_interface<maybe<T>> {
    private:
        using this_type = maybe;
        using super_type = std::optional<T>;

        using iterator = egret_detail::maybe_iter_impl::maybe_iterator<T>;
        using const_iterator = egret_detail::maybe_iter_impl::maybe_iterator<const T>;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        using super_type::super_type;

        this_type& operator =(const this_type&) = delete;
        constexpr this_type& operator =(this_type&&) noexcept(std::is_nothrow_move_assignable_v<super_type>) = delete;

    // -------------------------------------------------------------------------
    //  range behavior
    //
        constexpr iterator begin() noexcept { return this->has_value() ? iterator(std::ref(**this)) : iterator(); }
        constexpr const_iterator cbegin() const noexcept { return this->has_value() ? const_iterator(std::cref(**this)) : const_iterator(); }
        constexpr const_iterator begin() const noexcept { return this->cbegin(); }

        constexpr iterator end() noexcept { return iterator(); }
        constexpr const_iterator cend() const noexcept { return const_iterator(); }
        constexpr const_iterator end() const noexcept { return this->cend(); }

        constexpr std::size_t size() const noexcept { return this->has_value() ? 1 : 0; }

    // -------------------------------------------------------------------------
    //  monadic behavior
    //
        using super_type::operator bool;
        using super_type::has_value;
        using super_type::operator *;
        using super_type::operator->;
        using super_type::value;
        using super_type::value_or;
        using super_type::and_then;
        using super_type::transform;
        using super_type::or_else;

    }; // class maybe

    template <typename T>
    maybe(T) -> maybe<T>;

    template <typename T>
    maybe(std::reference_wrapper<T>) -> maybe<const T&>;

// -----------------------------------------------------------------------------
//  [class] maybe<T&>
// -----------------------------------------------------------------------------
    template <typename T>
    class maybe<const T&> : private std::optional<std::reference_wrapper<const T>> {
    private:
        using this_type = maybe;
        using super_type = std::optional<std::reference_wrapper<const T>>;

        using const_iterator = egret_detail::maybe_iter_impl::maybe_iterator<const T>;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        constexpr maybe() noexcept = default;
        constexpr maybe(const this_type&) noexcept = default;
        constexpr maybe(this_type&&) noexcept = default;

        constexpr maybe(std::nullopt_t) noexcept: super_type() {}

        template <typename U>
            requires requires (U&& obj) {
                { std::cref(std::forward<U>(obj)) } -> std::convertible_to<std::reference_wrapper<const T>>;
            }
        constexpr maybe(U&& obj)
            : super_type(std::cref(std::forward<U>(obj)))
        {
        }

        this_type& operator =(const this_type&) noexcept = delete;
        this_type& operator =(this_type&&) noexcept = delete;

    // -------------------------------------------------------------------------
    //  range behavior
    //
        constexpr const_iterator cbegin() const noexcept { return this->has_value() ? const_iterator(std::cref(**this)) : const_iterator(); }
        constexpr const_iterator begin() const noexcept { return this->cbegin(); }

        constexpr const_iterator cend() const noexcept { return const_iterator(); }
        constexpr const_iterator end() const noexcept { return this->cend(); }

        constexpr std::size_t size() const noexcept { return this->has_value() ? 1 : 0; }

    // -------------------------------------------------------------------------
    //  monadic behavior
    //
        using super_type::operator bool;
        using super_type::has_value;

        constexpr const T& operator *() const noexcept { return (super_type::operator *()).get(); }
        constexpr const T* operator->() const noexcept { return std::addressof(**this); }
        constexpr const T& value() const { return (super_type::value()).get(); }

        template <std::convertible_to<const T&> U>
        constexpr const T& value_or(U&& other_ref) const noexcept
        {
            return this->has_value() ? **this : static_cast<const T&>(other_ref);
        }

        template <std::invocable<const T&> F>
            requires std::default_initializable<std::remove_cvref_t<std::invoke_result_t<F, const T&>>>
        constexpr auto and_then(F&& f) const
            -> std::remove_cvref_t<std::invoke_result_t<F, const T&>>
        {
            return this->has_value()
                ? std::invoke(std::forward<F>(f), **this)
                : std::remove_cvref_t<std::invoke_result_t<F, const T&>> {};
        }

        template <std::invocable<const T&> F>
        constexpr auto transform(F&& f) const
        {
            using result_value_t = std::remove_cv_t<std::invoke_result_t<F, const T&>>;
            static_assert(!std::is_same_v<result_value_t, std::nullopt_t> && !std::is_same_v<result_value_t, std::in_place_t>);
            static_assert(std::is_object_v<result_value_t> && !std::is_array_v<result_value_t>);

            using result_t = std::optional<result_value_t>;
            return this->has_value()
                ? result_t(std::invoke(std::forward<F>(f), **this))
                : result_t{};
        }

        template <std::invocable F>
            requires std::convertible_to<std::invoke_result_t<F>, this_type>
        constexpr this_type or_else(F&& f) const
        {
            return this->has_value()
                ? *this
                : std::invoke(std::forward<F>(f));
        }

    }; // class maybe

} // namespace egret::util