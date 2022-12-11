#pragma once

#include <optional>
#include <type_traits>
#include <functional>
#include "concepts.h"
#include "interface.h"
#include "transformed.h"

#if __cpp_lib_optional >= 202110L
namespace egret_detail::j2obj_opt_impl {
// -----------------------------------------------------------------------------
//  [struct] opt_transformed_func
//  [struct] opt_and_then_func
//  [struct] opt_or_else_func
//  [struct] opt_value_or_func
// -----------------------------------------------------------------------------
    template <typename F>
    struct opt_transformed_func {
        template <typename Target>
        constexpr auto operator()(std::optional<Target>& opt) const { return opt.transform(f); }
        template <typename Target>
        constexpr auto operator()(const std::optional<Target>& opt) const { return opt.transform(f); }
        template <typename Target>
        constexpr auto operator()(std::optional<Target>&& opt) const { return std::move(opt).transform(f); }
        template <typename Target>
        constexpr auto operator()(const std::optional<Target>&& opt) const { return std::move(opt).transform(f); }
        F f;
    };
    template <typename F>
    struct opt_and_then_func {
        template <typename Target>
        constexpr auto operator()(std::optional<Target>& opt) const { return opt.and_then(f); }
        template <typename Target>
        constexpr auto operator()(const std::optional<Target>& opt) const { return opt.and_then(f); }
        template <typename Target>
        constexpr auto operator()(std::optional<Target>&& opt) const { return std::move(opt).and_then(f); }
        template <typename Target>
        constexpr auto operator()(const std::optional<Target>&& opt) const { return std::move(opt).and_then(f); }
        F f;
    };
    template <typename F>
    struct opt_or_else_func {
        template <typename Target>
        constexpr auto operator()(const std::optional<Target>& opt) const { return opt.or_else(f); }
        template <typename Target>
        constexpr auto operator()(std::optional<Target>&& opt) const { return std::move(opt).or_else(f); }
        F f;
    };
    template <typename T>
    struct opt_value_or_func {
        template <typename Target>
        constexpr auto operator()(const std::optional<Target>& opt) const { return opt.value_or(value); }
        template <typename Target>
        constexpr auto operator()(std::optional<Target>&& opt) const { return std::move(opt).value_or(value); }
        T value;
    };

} // namespace egret_detail::j2obj_opt_impl
#endif

namespace egret::util::j2obj {
// -----------------------------------------------------------------------------
//  [class] optional_deserializer
// -----------------------------------------------------------------------------
    template <typename Base>
    class optional_deserializer : public json_deserializer_interface<optional_deserializer<Base>> {
    private:
        using this_type = optional_deserializer;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        optional_deserializer() = delete;
        constexpr optional_deserializer(const this_type&)
            requires std::is_copy_constructible_v<Base> = default;
        constexpr optional_deserializer(this_type&&)
            noexcept(std::is_nothrow_move_constructible_v<Base>)
            requires std::is_move_constructible_v<Base> = default;

        explicit constexpr optional_deserializer(const Base& base)
            requires std::is_copy_constructible_v<Base>
            : base_(base)
        {
        }

        explicit constexpr optional_deserializer(Base&& base)
            noexcept(std::is_nothrow_move_constructible_v<Base>)
            requires std::is_move_constructible_v<Base>
            : base_(std::move(base))
        {
        }

    // -------------------------------------------------------------------------
    //  operator()
    //
        template <cpt::deserializable_json_with<Base> Json>
        auto operator()(const Json& json) const
            -> std::optional<deserialize_result_value_t<Base, Json>>
        {
            using result_t = std::optional<deserialize_result_value_t<Base, Json>>;
            if (json.is_null()) {
                return result_t(std::nullopt);
            }
            else {
                return result_t(std::invoke(base_, json));
            }
        }

    // -------------------------------------------------------------------------
    //  optional
    //
        constexpr this_type optional() const { return *this; }

    // -------------------------------------------------------------------------
    //  monadic behavior
    //
#if __cpp_lib_optional >= 202110L
        template <typename Self, typename F>
        constexpr auto transform(this Self&& self, F&& f)
        {
            using func_t = egret_detail::j2obj_opt_impl::opt_transformed_func<std::remove_cvref_t<F>>;
            using result_t = transformed_deserializer<this_type, func_t>;
            return result_t(std::forward<Self>(self), func_t(std::forward<F>(f)));
        }
        template <typename Self, typename F>
        constexpr auto and_then(this Self&& self, F&& f)
        {
            using func_t = egret_detail::j2obj_opt_impl::opt_and_then_func<std::remove_cvref_t<F>>;
            using result_t = transformed_deserializer<this_type, func_t>;
            return result_t(std::forward<Self>(self), func_t(std::forward<F>(f)));
        }
        template <typename Self, typename F>
        constexpr auto or_else(this Self&& self, F&& f)
        {
            using func_t = egret_detail::j2obj_opt_impl::opt_or_else_func<std::remove_cvref_t<F>>;
            using result_t = transformed_deserializer<this_type, func_t>;
            return result_t(std::forward<Self>(self), func_t(std::forward<F>(f)));
        }
        template <typename Self, typename T>
        constexpr auto value_or(this Self&& self, T&& value)
        {
            using func_t = egret_detail::j2obj_opt_impl::opt_value_or_func<std::remove_cvref_t<T>>;
            using result_t = transformed_deserializer<this_type, func_t>;
            return result_t(std::forward<Self>(self), func_t(std::forward<T>(value)));
        }
#endif

    // -------------------------------------------------------------------------
    //  get
    //
        constexpr const Base& base_deserializer() const noexcept { return base_; }

    private:
        Base base_;

    }; // class optional_deserializer

// -----------------------------------------------------------------------------
//  [class] json_deserializer_interface
// -----------------------------------------------------------------------------
    template <typename Derived>
    template <typename Self>
    constexpr auto json_deserializer_interface<Derived>::optional(this Self&& self)
        -> j2obj::optional_deserializer<Derived>
    {
        using result_t = j2obj::optional_deserializer<Derived>;
        return result_t{j2obj::cast(std::forward<Self>(self))};
    }

} // namespace egret::util::j2obj
