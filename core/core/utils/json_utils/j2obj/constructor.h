#pragma once

#include <type_traits>
#include <utility>
#include <tuple>
#include "core/concepts/non_reference.h"
#include "core/concepts/qualified.h"
#include "concepts.h"
#include "interface.h"

namespace egret::util::j2obj {
// -----------------------------------------------------------------------------
//  [class] constructor
// -----------------------------------------------------------------------------
    template <typename Target, typename ...T>
    class constructor : public json_deserializer_interface<constructor<Target, T...>> {
    private:
        using this_type = constructor;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        constructor() = delete;
        constexpr constructor(const this_type&)
            requires std::is_copy_constructible_v<std::tuple<T...>> = default;
        constexpr constructor(this_type&&)
            noexcept(std::is_nothrow_move_constructible_v<std::tuple<T...>>)
            requires std::is_move_constructible_v<std::tuple<T...>> = default;

        explicit constexpr constructor(const std::tuple<T...>& deserializers)
            requires std::is_copy_constructible_v<std::tuple<T...>>
            : deserializers_(deserializers)
        {
        }

        explicit constexpr constructor(std::tuple<T...>&& deserializers)
            requires std::is_move_constructible_v<std::tuple<T...>>
            : deserializers_(std::move(deserializers))
        {
        }

        template <cpt::qualified<T> ...AT>
            requires std::conjunction_v<std::is_constructible<T, AT>...>
        explicit(1 < sizeof...(AT)) constexpr constructor(AT&& ...deserializers)
            noexcept(std::conjunction_v<std::is_nothrow_constructible<T, AT>...>)
            : deserializers_(std::forward<AT>(deserializers)...)
        {
        }

        constexpr this_type& operator =(const this_type&)
            requires std::is_copy_assignable_v<std::tuple<T...>> = default;
        constexpr this_type& operator =(this_type&&)
            noexcept(std::is_nothrow_move_assignable_v<std::tuple<T...>>)
            requires std::is_move_assignable_v<std::tuple<T...>> = default;

    // -------------------------------------------------------------------------
    //  operator()
    //
        template <cpt::basic_json Json>
            requires 
                (cpt::deserializable_json_with<Json, T> && ...) &&
                std::is_constructible_v<Target, deserialize_result_t<T, Json>...>
        Target operator()(const Json& j) const
        {
            try {
                const auto ctor = [&j](const auto& ...d) {
                    return Target(d(j)...);
                };
                return std::apply(ctor, deserializers_);
            }
            catch (const std::exception& e) {
                throw exception("During deserializing json into {}, an exception occurs:\n{}",
                    nameof::nameof_type<Target>(),
                    e.what()
                ).record_stacktrace();
            }
            catch (...) {
                throw exception("During deserializing json into {}, an exception occurs.",
                    nameof::nameof_type<Target>()
                ).record_stacktrace();
            }
        }
        
    // -------------------------------------------------------------------------
    //  get
    //
        constexpr const std::tuple<T...>& base_deserializers() const noexcept { return deserializers_; }

    private:    
        std::tuple<T...> deserializers_;

    }; // class constructor

// -----------------------------------------------------------------------------
//  [class] construct_t
//  [value] construct
// -----------------------------------------------------------------------------
    template <cpt::non_reference T>
    class construct_t {
    public:
        template <typename ...D>
        constexpr auto operator()(D&& ...deserializers)
            -> constructor<T, std::remove_cvref_t<D>...>
        {
            using result_t = constructor<T, std::remove_cvref_t<D>...>;
            return result_t{std::forward<D>(deserializers)...};
        }

    }; // class construct_t

    template <cpt::non_reference T>
    inline constexpr auto construct = construct_t<T> {};

} // namespace egret::util::j2obj
