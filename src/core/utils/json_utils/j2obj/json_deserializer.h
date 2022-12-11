#pragma once

#include <memory>
#include <typeindex>
#include <nlohmann/json_fwd.hpp>
#include "core/utils/maybe.h"
#include "concepts.h"

namespace egret::util::j2obj {
// -----------------------------------------------------------------------------
//  [class] json_deserializer
// -----------------------------------------------------------------------------
    template <typename Target, cpt::basic_json Json = nlohmann::json>
    class json_deserializer : public json_deserializer_interface<json_deserializer<Json>> {
    private:
        using this_type = json_deserializer;
        struct base {
            virtual ~base() = default;
            virtual Target apply(const Json& j) const = 0;
            virtual std::type_index type() const noexcept = 0;
            virtual const void* ptr() const noexcept = 0;
        };

        template <typename C>
        struct concrete : base {
            concrete(const C& obj) : obj(obj) {}
            concrete(C&& obj) noexcept(std::is_nothrow_move_constructible_v<C>) : obj(std::move(obj)) {}
            Target apply(const Json& j) const override { return obj(j); }
            std::type_index type() const noexcept override { return typeid(C); }
            const void* ptr() const noexcept override { return std::addressof(obj); }
            C obj;
        };

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        json_deserializer() = delete;
        json_deserializer(const this_type&) noexcept = default;
        json_deserializer(this_type&&) noexcept = default;

        template <cpt::json_deserializer_interface<Json> C>
            requires std::is_invocable_r_v<Target, const C&, const Json&>
        json_deserializer(C&& deserializer)
            : obj_(std::make_shared<concrete<std::remove_cvref_t<C>>>(std::forward<C>(deserializer)))
        {
        }

        this_type& operator =(const this_type&) noexcept = default;
        this_type& operator =(this_type&&) noexcept = default;

    // -------------------------------------------------------------------------
    //  operator()
    //
        Target operator()(const Json& j) const { return obj_->apply(j); }

    // -------------------------------------------------------------------------
    //  type erasure behavior
    //
        std::type_index type() const noexcept { return obj_->type();  }

        template <cpt::json_deserializer_interface<Json> C>
            requires std::is_invocable_r_v<Target, const C&, const Json&>
        util::maybe<const C&> as() const noexcept
        {
            const auto type = this->type();
            return type == typeid(C) 
                ? util::maybe<const C&>(*reinterpret_cast<const C*>(obj_->ptr()))
                : util::maybe<const C&>();
        }
        
    private:
        std::shared_ptr<const base> obj_;

    }; // class json_deserializer

} // namespace egret::util::j2obj
