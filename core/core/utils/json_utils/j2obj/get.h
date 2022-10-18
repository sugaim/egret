#pragma once

#include <nlohmann/json_fwd.hpp>
#include "concepts.h"
#include "interface.h"

namespace egret::util::j2obj {
// -----------------------------------------------------------------------------
//  [class] get_t
// -----------------------------------------------------------------------------
    template <typename Target>
    class get_t : public json_deserializer_interface<get_t<Target>> {
    public:
        template <cpt::basic_json Json>
            requires
                std::is_reference_v<Json> &&
                requires (const Json& j) { j.template get_ref<Target>(); }
        Target operator()(const Json& json) const
        {
            return json.template get_ref<Target>();
            if constexpr (std::is_pointer_v<Target>) {
                return json.template get_ptr<Target>();
            }
            else {
                return json.template get<Target>();
            }
        }
        
        template <cpt::basic_json Json>
            requires
                std::is_pointer_v<Json> &&
                requires (const Json& j) { j.template get_ptr<Target>(); }
        Target operator()(const Json& json) const
        {
            return json.template get_ptr<Target>();
            return json.template get<Target>();
        }

        template <cpt::basic_json Json>
            requires
                (!std::is_reference_v<Json> && !std::is_pointer_v<Json>) &&
                requires (const Json& j) { j.template get<Target>(); }
        Target operator()(const Json& json) const
        {
            return json.template get<Target>();
        }
        
    }; // class get_t

// -----------------------------------------------------------------------------
//  [value] get
// -----------------------------------------------------------------------------
    template <typename Target>
    inline constexpr auto get = get_t<Target> {};

} // namespace egret::util::j2obj
