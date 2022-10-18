#pragma once

#include <map>
#include "core/concepts/constructible_to.h"
#include "core/concepts/non_void.h"
#include "core/assertions/exception.h"
#include "core/utils/string_utils/to_string.h"
#include "interface.h"
#include "json_deserializer.h"

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

} // namespace egret::util::j2obj