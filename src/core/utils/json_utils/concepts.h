#pragma once

#include <nlohmann/json_fwd.hpp>

namespace egret_detail::jdesr_impl {
    template<
        template <typename, typename, typename...> typename Object,
        template <typename, typename...> typename Array,
        typename String, typename Boolean, typename Integer, typename Unsigned, typename Number,
        template <typename> typename Alloc,
        template <typename T, typename SFINAE> typename Serializer,
        typename Binary
    >
    void is_basic_json(const nlohmann::basic_json<
        Object, 
        Array, 
        String, Boolean, Integer, Unsigned, Number, 
        Alloc, 
        Serializer, 
        Binary
    >&) {}

} // namespace egret_detail::jdesr_impl

namespace egret::cpt {
// -----------------------------------------------------------------------------
//  [concept] basic_json
// -----------------------------------------------------------------------------
    template <typename J>
    concept basic_json = requires (const J& json) {
        egret_detail::jdesr_impl::is_basic_json(json);
    };

} // namespace egret::cpt
