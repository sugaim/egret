#pragma once

#include <tuple>
#include <nameof.hpp>
#include "core/assertions/assertion.h"
#include "concepts.h"
#include "interface.h"

namespace egret::util::j2obj {
// -----------------------------------------------------------------------------
//  [class] tuple
// -----------------------------------------------------------------------------
    template <typename ...T>
    class tuple : public json_deserializer_interface<tuple<T...>>, public std::tuple<T...> {
    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        using std::tuple<T...>::tuple;
        using std::tuple<T...>::operator =;

    // -------------------------------------------------------------------------
    //  operator()
    //
        template <cpt::basic_json Json>
            requires (cpt::deserializable_json_with<Json, T> && ...)
        auto operator()(const Json& j) const
            -> std::tuple<deserialize_result_value_t<T, Json>...>
        {
            using result_t = std::tuple<deserialize_result_value_t<T, Json>...>;
            try {
                const auto ctor = [&j](const auto& ...d) {
                    return result_t(d(j)...);
                };
                return std::apply(ctor, static_cast<const std::tuple<T...>&>(*this));
            }
            catch (const std::exception& e) {
                throw exception("During deserializing json into {}, an exception occurs:\n{}",
                    nameof::nameof_type<result_t>(),
                    e.what()
                ).record_stacktrace();
            }
            catch (...) {
                throw exception("During deserializing json into {}, an exception occurs.",
                    nameof::nameof_type<result_t>()
                ).record_stacktrace();
            }
        }

    // -------------------------------------------------------------------------
    //  get
    //
        constexpr const std::tuple<T...>& deserializers() const noexcept { return *this; }

    }; // class tuple

} // namespace egret::util::j2obj
