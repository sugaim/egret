#pragma once

#include <map>
#include "core/concepts/constructible_to.h"
#include "core/concepts/non_void.h"
#include "core/assertions/exception.h"
#include "core/utils/string_utils/to_string.h"
#include "interface.h"
#include "json_deserializer.h"

namespace egret_detail::j2obj_impl {
    template <typename Dispatcher, typename Json>
    using dispatcher_result_t = std::remove_cvref_t<std::invoke_result_t<const Dispatcher&, const Json&>>;

    template <typename Target, typename Json, typename Dispatcher, typename Less>
    using deserializer_map_t = std::map<
        dispatcher_result_t<Dispatcher, Json>,
        egret::util::j2obj::json_deserializer<Target, Json>,
        Less
    >;

} // namespace egret_detail::j2obj_impl;

namespace egret::util::j2obj {
// -----------------------------------------------------------------------------
//  [class] dispatchable_deserializer
// -----------------------------------------------------------------------------
    template <typename Target, cpt::basic_json Json, typename Dispatcher, typename Less>
        requires requires (const Dispatcher& d, const Json& j) {
            { d(j) } -> cpt::non_void;
        }
    class dispatchable_deserializer : public json_deserializer_interface<dispatchable_deserializer<Target, Json, Dispatcher, Less>> {
    private:
        using this_type = dispatchable_deserializer;
        using key_type = std::remove_cvref_t<std::invoke_result_t<const Dispatcher&, const Json&>>;
        using value_type = json_deserializer<Target, Json>;
        using map_type = std::map<key_type, value_type, Less>;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        dispatchable_deserializer() = delete;
        dispatchable_deserializer(const this_type&) = default;
        dispatchable_deserializer(this_type&&) = default;

        template <cpt::constructible_to<Dispatcher> D, cpt::constructible_to<map_type> M>
        dispatchable_deserializer(D&& dispatcher, M&& deserializer_map)
            : dispatcher_(std::forward<D>(Dispatcher)),
              map_(std::forward<M>(deserializer_map))
        {
        }

        this_type& operator =(const this_type&) = default;
        this_type& operator =(this_type&&) = default;

    // -------------------------------------------------------------------------
    //  operator()  
    //
        Target operator()(const Json& j) const
        {
            const auto& key = dispatcher_(j);
            if (!map_.contains(key)) {
                if constexpr (cpt::to_stringable<key_type>) {
                    std::string supported_types = "";
                    auto inserter = std::back_inserter(supported_types);
                    for (const auto& [k, _] : map_) {
                        inserter = std::format_to(inserter, "'{}', ", k);
                    }
                    throw exception(
                        "dispatchable json deserializer does not support key='{}'. "
                        "[supported={}]", 
                        key, supported_types
                    ).record_stacktrace();
                }
                else {
                    throw exception(
                        "dispatchable json deserializer does not support key='{}'. "
                        "[supported={}]", 
                        key, supported_types
                    ).record_stacktrace();
                }
            }
        }

    // -------------------------------------------------------------------------
    //  get
    //
        const Dispatcher& dispatcher() const noexcept { return dispatcher_; }
        const map_type& deserializer_map() const noexcept { return map_; }

    private:
        Dispatcher dispatcher_;
        map_type map_;

    }; // class dispatchable_deserializer

// -----------------------------------------------------------------------------
//  [fn] make_dispatchable
// -----------------------------------------------------------------------------
    template <
        typename Target, cpt::basic_json Json = nlohmann::json, 
        typename Dispatcher, typename Less = std::less<>
    >
        requires requires (const Json& j, const Dispatcher& d) {
            { d(j) } -> cpt::non_void;
        }
    auto make_dispatchable(
        Dispatcher&& dispatcher,
        egret_detail::j2obj_impl::deserializer_map_t<Target, Json, Dispatcher, Less> map
    ) -> dispatchable_deserializer<Target, Json, std::remove_cvref_t<Dispatcher>, Less>
    {
        using result_t = dispatchable_deserializer<Target, Json, std::remove_cvref_t<Dispatcher>, Less>;
        return result_t{std::forward<Dispatcher>(dispatcher), std::move(map)};
    }

} // namespace egret::util::j2obj
