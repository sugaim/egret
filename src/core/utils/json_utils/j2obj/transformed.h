#pragma once

#include "concepts.h"
#include "interface.h"

namespace egret::util::j2obj {
// -----------------------------------------------------------------------------
//  [class] transformed_deserializer
// -----------------------------------------------------------------------------
    template <typename Base, typename F>
    class transformed_deserializer : public json_deserializer_interface<transformed_deserializer<Base, F>> {
    private:
        using this_type = transformed_deserializer;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        transformed_deserializer() = delete;
        constexpr transformed_deserializer(const this_type&)
            requires std::is_copy_constructible_v<Base> && std::is_copy_constructible_v<F> = default;
        constexpr transformed_deserializer(this_type&&)
            noexcept(std::is_nothrow_move_constructible_v<Base> && std::is_nothrow_move_constructible_v<F>)
            requires std::is_move_constructible_v<Base> && std::is_move_constructible_v<F> = default;

        constexpr transformed_deserializer(const Base& base, F f)
            requires std::is_copy_constructible_v<Base> && std::is_move_constructible_v<F>
            : base_(base), f_(std::move(f))
        {
        }

        constexpr transformed_deserializer(Base&& base, F f)
            noexcept(std::is_nothrow_move_constructible_v<Base> && std::is_nothrow_move_constructible_v<F>)
            requires std::is_move_constructible_v<Base> && std::is_move_constructible_v<F>
            : base_(std::move(base)), f_(std::move(f))
        {
        }
        
    // -------------------------------------------------------------------------
    //  operator()
    //
        template <cpt::deserializable_json_with<Base> Json>
            requires std::invocable<const F&, deserialize_result_t<Base, Json>>
        auto operator()(const Json& json) const
            -> std::invoke_result_t<const F&, deserialize_result_t<Base, Json>>
        {
            return std::invoke(f_, base_(json));
        }

    // -------------------------------------------------------------------------
    //  get
    //
        constexpr const Base& base_deserializer() const noexcept { return base_; }
        constexpr const F& function() const noexcept { return f_; }

    private:
        Base base_;
        F f_;
        
    }; // class transformed_deserializer

// -----------------------------------------------------------------------------
//  [class] json_deserializer_interface
// -----------------------------------------------------------------------------
    template <typename Derived>
    template <typename Self, typename F>
    constexpr auto json_deserializer_interface<Derived>::transform(this Self&& self, F&& f)
        -> transformed_deserializer<Derived, std::remove_cvref_t<F>>
    {
        using result_t = transformed_deserializer<Derived, std::remove_cvref_t<F>>;
        return result_t{j2obj::cast(std::forward<Self>(self)), std::forward<F>(f)};
    }

    template <typename Derived>
    template <typename T, typename Self>
    constexpr auto json_deserializer_interface<Derived>::to(this Self&& self)
        -> transformed_deserializer<Derived, util::construct_t<T>>
    {
        return std::forward<Self>(self).transform(util::construct<T>);
    }

} // namespace egret::util::j2obj
