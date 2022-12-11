#pragma once

#include <tuple>
#include <variant>
#include <typeindex>
#include <ranges>
#include <array>
#include <nameof.hpp>
#include "core/concepts/qualified.h"
#include "core/assertions/exception.h"
#include "core/type_traits/special_functions_properties.h"
#include "concepts.h"
#include "interface.h"

namespace egret::util::j2obj {
// -----------------------------------------------------------------------------
//  [class] variant
// -----------------------------------------------------------------------------
    template <typename Target, typename TI, typename ...Base>
    class variant : public json_deserializer_interface<variant<Target, TI, Base...>> {
    private:
        using this_type = variant;
        using spfn_props = special_function_properties<TI, Base...>;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns 
    //
        variant() = delete;
        constexpr variant(const this_type&)
            requires spfn_props::are_copy_constructible_v = default;
        constexpr variant(this_type&&)
            noexcept(spfn_props::are_nothrow_move_constructible_v)
            requires spfn_props::are_move_constructible_v = default;

        template <cpt::qualified<TI> ATI, cpt::qualified<Base> ...ABase>
        constexpr variant(ATI&& ti, ABase&& ...base)
            noexcept(spfn_props::template is_each_nothrow_constructible_from_v<ATI, ABase...>)
            requires spfn_props::template is_each_constructible_from_v<ATI, ABase...>
            : type_identifier_(std::forward<ATI>(ti)), base_(std::forward<ABase>(base)...)
        {
        }

        constexpr this_type& operator =(const this_type&)
            requires spfn_props::are_copy_assignable_v = default;
        constexpr this_type& operator =(this_type&&)
            noexcept(spfn_props::are_nothrow_move_assignable_v)
            requires spfn_props::are_move_assignable_v = default;

    // -------------------------------------------------------------------------
    //  operator()
    //
    private:
        template <typename ResultType, std::size_t i, cpt::basic_json Json>
        ResultType deserialize_impl(const Json& json, std::type_index type) const
        {
            if constexpr (i == sizeof...(Base)) {
                constexpr std::array type_names {
                    nameof::nameof_type<deserialize_result_value_t<Base, Json>>()...
                };
                constexpr auto joined_type_names = type_names | std::views::join_with(std::string_view(", "));
                throw exception(
                    "Unsupported type is detected. [given={}, supported={{{}}}]",
                    type.name(), std::string(joined_type_names.begin(), joined_type_names.end())
                ).record_stacktrace();
            }
            else {
                using ith_result_t = deserialize_result_value_t<std::tuple_element_t<i, std::tuple<Base...>>, Json>;
                if (typeid(ith_result_t) == type) {
                    return ResultType (std::get<i>(base_)(json));
                }
                else {
                    return this->deserialize_impl<i+1>(json, type);
                }
            }
        }

    public:
        template <cpt::basic_json Json>
            requires
                std::is_invocable_r_v<std::type_index, const TI&, const Json&> &&
                (cpt::deserializable_json_with<Json, Base> && ...) && (
                    std::is_void_v<Target> ||
                    std::disjunction_v<std::is_constructible<Target, deserialize_result_t<Base, Json>>...>
                )
        auto operator()(const Json& json) const
            -> std::conditional_t<
                std::is_void_v<Target>,
                std::variant<deserialize_result_value_t<Base, Json>...>,
                Target
            >
        {
            using result_t = std::conditional_t<
                std::is_void_v<Target>,
                std::variant<deserialize_result_value_t<Base, Json>...>,
                Target
            >;
            const std::type_index type = std::invoke(type_identifier_, json);
            return this->deserialize_impl<result_t, 0>(json, type);
        }

    // -------------------------------------------------------------------------
    //  get
    //
        constexpr const TI& type_identifier() const noexcept { return type_identifier_; }
        constexpr const std::tuple<Base...>& deserializers() const noexcept { return base_; }

    private:
        TI type_identifier_;
        std::tuple<Base...> base_;

    }; // class variant

} // namespace egret::util::j2obj
