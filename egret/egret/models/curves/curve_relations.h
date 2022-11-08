#pragma once

#include <map>
#include <string>
#include <vector>
#include <nlohmann/json_fwd.hpp>
#include "core/assertions/assertion.h"
#include "core/utils/json_utils/j2obj.h"
#include "core/utils/string_utils/to_string.h"
#include "composite_curve.h"

namespace egret::model {
// -----------------------------------------------------------------------------
//  [struct] component_curve_identifier 
// -----------------------------------------------------------------------------
    struct component_curve_identifier {
        std::string name;
        double multiplier;
    };

// -----------------------------------------------------------------------------
//  [class] curve_relations
// -----------------------------------------------------------------------------
    template <typename CurveKey>
    class curve_relations : private std::map<CurveKey, std::vector<component_curve_identifier>, std::ranges::less> {
    private:
        using this_type = curve_relations;
        using super_type = std::map<CurveKey, std::vector<component_curve_identifier>, std::ranges::less>;

    public:
        using super_type::super_type;
        using super_type::operator =;

        using super_type::operator[];
        using super_type::at;

        using super_type::emplace;
        using super_type::try_emplace;

        using super_type::find;
        using super_type::contains;

        using super_type::begin;
        using super_type::end;
        using super_type::cbegin;
        using super_type::cend;
        using super_type::rbegin;
        using super_type::rend;
        using super_type::size;
        using super_type::empty;
        
        void swap(this_type& other) noexcept { super_type::swap(static_cast<super_type&>(other)); }
        void merge(this_type&  other) { super_type::merge(static_cast<super_type&>(other)); }
        void merge(this_type&& other) { super_type::merge(static_cast<super_type&&>(other)); }

    // -------------------------------------------------------------------------
    //  create
    //
    private:
        template <typename Key>
        void composition_existence_validation(const Key& curve_tag) const
        {
            if constexpr (cpt::to_stringable<Key>) {
                egret::assertion(
                    this->contains(curve_tag), 
                    "Curve is tried to build, but composition data is not found for {}.", curve_tag
                );
            }
            else {
                egret::assertion(
                    this->contains(curve_tag), 
                    "Curve is tried to build, but composition data is not found."
                );
            }
        }
        template <typename Key, cpt::yield_curve C, typename Cmp, typename Alloc>
        static void component_existence_validation(
            const Key& curve_tag, const std::string& name,
            const std::map<std::string, C, Cmp, Alloc>& component_curves
        )
        {
            if constexpr (cpt::to_stringable<Key>) {
                egret::assertion(
                    component_curves.contains(name), 
                    "Component curve is not found. [curve={}, component={}]", curve_tag, name
                );
            }
            else {
                egret::assertion(
                    component_curves.contains(name), 
                    "Component curve is not found. [component={}]", name
                );
            }
        }

    public:
        template <typename Key, cpt::yield_curve C, typename Cmp, typename Alloc>
            requires std::strict_weak_order<std::ranges::less, CurveKey, Key>
        composite_curve<C> create(
            const Key& curve_tag,
            const std::map<std::string, C, Cmp, Alloc>& component_curves
        ) const
        {
            this->composition_existence_validation(curve_tag);
            const std::vector<component_curve_identifier>& identifiers = *(this->find(curve_tag));
            std::vector<curve_component<C>> components;
            components.reserve(identifiers.size());
            for (const auto& identifier : identifiers) {
                component_existence_validation(curve_tag, identifier.name, component_curves);
                components.emplace_back(
                    identifier.name,
                    identifier.multiplier,
                    component_curves.at(identifier.name)
                );
            }
            return composite_curve<C>(std::move(components));
        }

    }; // class curve_system

} // namespace egret::model

namespace nlohmann {
    template <>
    struct adl_serializer<egret::model::component_curve_identifier> {

        using target_type = egret::model::component_curve_identifier;

        template <typename Json>
        static target_type from_json(const Json& j)
        {
            namespace j2obj = egret::util::j2obj;
            constexpr auto deser = j2obj::construct<target_type>(
                "name" >> j2obj::string,
                "multiplier" >> j2obj::number
            );
            return deser(j);
        }
        template <typename Json>
        static void to_json(Json& j, const target_type& obj)
        {
            j["name"] = obj.name;
            j["multipler"] = obj.multiplier;
        }
    };

    template <typename CurveKey>
    struct adl_serializer<egret::model::curve_relations<CurveKey>> {

        using target_type = egret::model::curve_relations<CurveKey>;

        template <typename Json>
        static target_type from_json(const Json& j)
        {
            namespace j2obj = egret::util::j2obj;
            constexpr build_components = j2obj::array.to_range_of<egret::model::component_curve_identifier>
                | std::ranges::to<std::vector>();

            egret::assertion(j.is_array(), "Json must be an array, whose elements are expected as an curve relation.");
            target_type result {};
            for (const auto& single_relation : j) {
                auto key = ("curve" >> j2obj::get<CurveKey>)(single_relation);
                auto components = ("components" >> build_components)(single_relation);
                result.try_emplace(std::move(key), std::move(components));
            }
            return result;
        }
        template <typename Json>
        static void to_json(Json& j, const target_type& obj)
        {
            j = Json::array();
            for (const auto& [key, components] : obj) {
                j.push_back({
                    {"curve", key},
                    {"components", components}
                });
            }
        }
    };

} // namespace nlohmann
