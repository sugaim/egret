#pragma once

#include "core/assertions/assertion.h"
#include "core/concepts/qualified.h"
#include "concepts.h"
#include "interface.h"

namespace egret::util::j2obj {
// -----------------------------------------------------------------------------
//  [class] property
// -----------------------------------------------------------------------------
    template <typename String, typename Base>
    class property : public json_deserializer_interface<property<String, Base>> {
    private:
        using this_type = property;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        property() = delete;
        constexpr property(const this_type&)
            requires std::is_copy_constructible_v<String> &&
                     std::is_copy_constructible_v<Base> = default;
        constexpr property(this_type&&)
            noexcept(std::is_nothrow_move_constructible_v<String> &&
                     std::is_nothrow_move_constructible_v<Base>)
            requires std::is_move_constructible_v<String> &&
                     std::is_move_constructible_v<Base> = default;
            
        template <cpt::qualified<String> S, cpt::qualified<Base> B>
        constexpr property(S&& name, B&& base_deserializer)
            noexcept(std::is_nothrow_constructible_v<String, S> &&
                     std::is_nothrow_constructible_v<Base, B>)
            requires std::is_constructible_v<String, S> &&
                     std::is_constructible_v<Base, B>
            : name_(std::forward<S>(name)),
              base_(std::forward<B>(base_deserializer))
        {
        }

        constexpr this_type& operator =(const this_type&)
            requires std::is_copy_assignable_v<String> &&
                     std::is_copy_assignable_v<Base> = default;
        constexpr this_type& operator =(const this_type&)
            noexcept(std::is_nothrow_move_assignable_v<String> &&
                     std::is_nothrow_move_assignable_v<Base>)
            requires std::is_move_assignable_v<String> &&
                     std::is_move_assignable_v<Base> = default;

    // -------------------------------------------------------------------------
    //  operator()
    //
        template <cpt::deserializable_json_with<Base> Json>
            requires requires (const Json& j, const String& s) {
                { j.contains(s) } -> std::convertible_to<bool>;
            }
        constexpr deserialize_result_t<Base, Json> operator()(const Json& j) const
        {
            egret::assertion(j.is_object(), "Json must be object type to get property.");
            try {
                if (j.contains(name_)) [[likely]] {
                    return base_(j[name_]);
                }
                else {
                    return base_(Json(nullptr));
                }
            }
            catch (const std::exception& e) {
                if constexpr (std::is_constructible_v<std::string_view, const String&>) {
                    throw exception("Fail to deserialize property '{}' because of:\n{}",
                        std::string_view(name_), e.what()
                    ).record_stacktrace();
                }
                else {
                    throw exception("Fail to deserialize property because of:\n{}", e.what()).record_stacktrace();
                }
            }
            catch (...) {
                if constexpr (std::is_constructible_v<std::string_view, const String&>) {
                    throw exception("An exception occured during deserializing property '{}'",
                        std::string_view(name_)
                    ).record_stacktrace();
                }
                else {
                    throw exception("An exception occured during deserializing property").record_stacktrace();
                }
            }
        }

    // -------------------------------------------------------------------------
    //  get  
    //
        constexpr const String& name() const noexcept { return name_; }
        constexpr const Base& deserializer() const noexcept { return base_; }

    private:
        String name_;
        Base base_;

    }; // class property

    template <typename S, typename D>
    property(S, D) -> property<S, D>;

    template <std::size_t n, typename D>
    constexpr auto operator>>(const char (&name)[n], const json_deserializer_interface<D>& base)
        -> property<std::string_view, D>
    {
        return property<std::string_view, D>(
            std::string_view(name), j2obj::cast(base));
    }

    template <std::size_t n, typename D>
    constexpr auto operator>>(const char (&name)[n], json_deserializer_interface<D>&& base)
        -> property<std::string_view, D>
    {
        return property<std::string_view, D>(
            std::string_view(name), j2obj::cast(std::move(base)));
    }

} // namespace egret::util::j2obj
