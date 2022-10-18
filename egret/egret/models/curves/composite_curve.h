#pragma once

#include <vector>
#include <tuple>
#include <array>
#include "core/type_traits/special_functions_properties.h"
#include "concepts.h"
#include "curve_component.h"

namespace egret::model {
// -----------------------------------------------------------------------------
//  [class] composite_curve
// -----------------------------------------------------------------------------
    template <cpt::yield_curve C>
    class composite_curve {
    private:
        using this_type = composite_curve;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        composite_curve() = delete;
        composite_curve(const this_type&) = default;
        composite_curve(this_type&&) noexcept = default;

        explicit composite_curve(const std::vector<curve_component<C>>& components)
            : components_(components)
        {
        }

        explicit composite_curve(std::vector<curve_component<C>>&& components)
            : components_(std::move(components))
        {
        }

        this_type& operator =(const this_type&) = default;
        this_type& operator =(this_type&&) noexcept = default;

    // -------------------------------------------------------------------------
    //  forward_rate
    //
        forward_rate_t<C> forward_rate(const std::chrono::sys_days& from, const std::chrono::sys_days& to) const
        {
            forward_rate_t<C> result = 0;
            for (const curve_component<C>& component : components_) {
                result += model::forward_rate(component.curve, from, to) * component.multiplier;
            }
            return result;
        }

    // -------------------------------------------------------------------------
    //  get
    //
        const std::vector<curve_component<C>>& components() const noexcept { return components_; }

        double multiplier_for(std::string_view name) const
        {
            double result = 0;
            for (const curve_component<C>& component : components_) {
                if (component.name == name) {
                    result += component.multiplier;
                }
            }
            return result;
        }

    private:
        std::vector<curve_component<C>> components_;

    }; // class composite_curve

// -----------------------------------------------------------------------------
//  [class] tuple_composite_curve
// -----------------------------------------------------------------------------
    template <cpt::yield_curve ...C>
    class tuple_composite_curve {
    private:
        using this_type = tuple_composite_curve;
        using components_type = std::tuple<curve_component<C>...>;
        using spfn_properties = special_function_properties<components_type>;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        tuple_composite_curve() = delete;
        tuple_composite_curve(const this_type&) = default;
        tuple_composite_curve(this_type&&) noexcept(std::is_nothrow_move_constructible_v<components_type>) = default;

        explicit tuple_composite_curve(const components_type& components)
            : components_(components)
        {
        }

        explicit tuple_composite_curve(components_type&& components)
            noexcept(std::is_nothrow_move_constructible_v<components_type>)
            : components_(std::move(components))
        {
        }

        this_type& operator =(const this_type&) = default;
        this_type& operator =(this_type&&) noexcept(std::is_nothrow_move_assignable_v<components_type>) = default;

    // -------------------------------------------------------------------------
    //  forward_rate
    //
        auto forward_rate(const std::chrono::sys_days& from, const std::chrono::sys_days& to) const
            -> std::common_type_t<double, forward_rate_t<C>...>
        {
            using result_t = std::common_type_t<double, forward_rate_t<C>...>;
            const auto calc_forward = [&from, &to](const auto& ...cmps) {
                return (
                    static_cast<result_t>(0.) + ... +
                    static_cast<result_t>(model::forward_rate(cmps.curve, from, to) * cmps.multiplier)
                );
            };
            return std::apply(calc_forward, components_);
        }

    // -------------------------------------------------------------------------
    //  get
    //
        const components_type& components() const noexcept { return components_; }

        double multiplier_for(std::string_view name) const
        {
            using pair_t = std::pair<std::string_view, double>;
            constexpr auto get_multipliers = [](const auto& ...cmps) {
                return std::array {pair_t{cmps.name, cmps.multiplier}...};
            };
            const auto multipliers = std::apply(get_multipliers, components_);

            double result = 0;
            for (const pair_t& component : multipliers) {
                if (component.first == name) {
                    result += component.second;
                }
            }
            return result;
        }

    private:
        components_type components_;

    }; // class tuple_composite_curve

} // namespace egret::model
