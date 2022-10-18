#pragma once

#include <map>
#include "core/concepts/qualified.h"
#include "core/assertions/assertion.h"
#include "concepts.h"
#include "interface.h"

namespace egret::util::j2obj {
// -----------------------------------------------------------------------------
//  [class] map
// -----------------------------------------------------------------------------
    template <typename KMapper, typename VDesir, typename Less = std::less<>>
    class map : public json_deserializer_interface<map<KMapper, VDesir, Less>> {
    private:
        using this_type = map;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        map() = delete;
        constexpr map(const this_type&)
            requires std::is_copy_constructible_v<KMapper> && std::is_copy_constructible_v<VDesir> = default;
        constexpr map(this_type&&)
            noexcept(std::is_nothrow_move_constructible_v<KMapper> &&
                     std::is_nothrow_move_constructible_v<VDesir> &&
                     std::is_nothrow_move_constructible_v<Less>)
            requires std::is_move_constructible_v<KMapper> &&
                     std::is_move_constructible_v<VDesir> &&
                     std::is_nothrow_move_constructible_v<Less> = default;

        template <cpt::qualified<KMapper> AKM, cpt::qualified<VDesir> AVD>
        constexpr map(AKM&& key_mapper, AVD&& value_deserializer, const Less& less = {})
            noexcept(std::is_nothrow_constructible_v<KMapper, AKM> &&
                     std::is_nothrow_constructible_v<VDesir, AVD> &&
                     std::is_nothrow_copy_constructible_v<Less>)
            requires std::is_constructible_v<KMapper, AKM> && 
                     std::is_constructible_v<VDesir, AVD> &&
                     std::is_copy_constructible_v<Less>
            : key_mapper_(std::forward<AKM>(key_mapper)),
              value_deser_(std::forward<AVD>(value_deserializer)),
              less_(less)
        {
        }

        constexpr this_type& operator =(const this_type&)
            requires std::is_copy_assignable_v<KMapper> && 
                     std::is_copy_assignable_v<VDesir> && 
                     std::is_copy_assignable_v<Less> = default;
        constexpr this_type& operator =(this_type&&)
            noexcept(std::is_nothrow_move_assignable_v<KMapper> &&
                     std::is_nothrow_move_assignable_v<VDesir> &&
                     std::is_nothrow_move_assignable_v<Less>)
            requires std::is_move_assignable_v<KMapper> && 
                     std::is_move_assignable_v<VDesir> && 
                     std::is_move_assignable_v<Less> = default;

    // -------------------------------------------------------------------------
    //  operator()
    //
        template <cpt::basic_json Json>
            requires
                std::is_invocable_v<const KMapper&, const typename Json::string_t&> &&
                std::is_invocable_v<const VDesir&, const Json&>
        auto operator()(const Json& json) const
            -> std::map<
                std::remove_cvref_t<std::invoke_result_t<const KMapper&, const typename Json::string_t&>>,
                std::remove_cvref_t<std::invoke_result_t<const VDesir&, const Json&>>, 
                Less
            >
        {
            egret::assertion(
                json.is_object(),
                "Json element is expected to be an object to construct a std::map, but actual data type is {}.",
                json.type_name()
            );
            using key_type = std::remove_cvref_t<std::invoke_result_t<const KMapper&, const typename Json::string_t&>>;
            using value_type = std::remove_cvref_t<std::invoke_result_t<const VDesir&, const Json&>>;
            std::map<key_type, value_type, Less> result (less_);
            for (const auto& [key, value_json] : json.items()) {
                result.emplace(
                    std::invoke(key_mapper_, key),
                    value_deser_(value_json)
                );
            }
            return result;
        }

    // -------------------------------------------------------------------------
    //  get  
    //
        constexpr const KMapper& key_mapper() const noexcept { return key_mapper_; }
        constexpr const VDesir& value_deserializer() const noexcept { return value_deser_; }
        constexpr const Less& less() const noexcept { return less_; }
        
    private:
        KMapper key_mapper_;
        VDesir value_deser_;
        Less less_;

    }; // class map

    template <typename KeyFunc, typename ValueDeser>
    map(KeyFunc, ValueDeser) -> map<KeyFunc, ValueDeser>;

    template <typename KeyFunc, typename ValueDeser, typename Less>
    map(KeyFunc, ValueDeser, Less) -> map<KeyFunc, ValueDeser, Less>;

} // namespace egret::util::j2obj
