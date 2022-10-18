#pragma once

#include "concepts.h"
#include "core/utils/construct.h"

namespace egret::util::j2obj {
// -----------------------------------------------------------------------------
//  [class] optional_deserializer
//  [class] transformed_deserializer
// -----------------------------------------------------------------------------
    template <typename Base>
    class optional_deserializer;

    template <typename Base, typename F>
    class transformed_deserializer;

// -----------------------------------------------------------------------------
//  [class] json_deserializer_interface
// -----------------------------------------------------------------------------
    template <typename Derived>
    class json_deserializer_interface {
    public:
        template <typename Self>
        constexpr auto optional(this Self&& self) 
            -> j2obj::optional_deserializer<Derived>;

        template <typename Self, typename F>
        constexpr auto transform(this Self&& self, F&& f)
            -> j2obj::transformed_deserializer<Derived, std::remove_cvref_t<F>>;

        template <typename T, typename Self>
        constexpr auto to(this Self&& self)
            -> j2obj::transformed_deserializer<Derived, util::construct_t<T>>;

    }; // class json_deserializer_interface

    template <typename D>
    constexpr D& cast(json_deserializer_interface<D>& deser) noexcept
    {
        return static_cast<D&>(deser);
    }
    template <typename D>
    constexpr const D& cast(const json_deserializer_interface<D>& deser) noexcept
    {
        return static_cast<const D&>(deser);
    }
    template <typename D>
    constexpr D&& cast(json_deserializer_interface<D>&& deser) noexcept
    {
        return static_cast<D&&>(deser);
    }
    template <typename D>
    constexpr const D&& cast(const json_deserializer_interface<D>&& deser) noexcept
    {
        return static_cast<const D&&>(deser);
    }

} // namespace egret::util::j2obj