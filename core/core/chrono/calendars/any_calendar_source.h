#pragma once

#include <memory>
#include <typeindex>
#include "core/concepts/different_from.h"
#include "core/utils/maybe.h"
#include "concepts.h"

namespace egret::chrono {
// -----------------------------------------------------------------------------
//  [class] any_calendar_source
// -----------------------------------------------------------------------------
    class any_calendar_source {
    private:
        using this_type = any_calendar_source;

        struct base {
            virtual ~base() = default;
            virtual std::optional<std::vector<std::chrono::sys_days>> get_additional_holidays(std::string_view code) const = 0;
            virtual std::optional<std::vector<std::chrono::sys_days>> get_additional_businessdays(std::string_view code) const = 0;
            virtual std::type_index type() const noexcept = 0;
            virtual const void* pointer() const noexcept = 0;
        };

        template <cpt::calendar_source C>
        struct concrete final : base {
            concrete(C obj) noexcept(std::is_nothrow_move_constructible_v<C>): obj_(std::move(obj)) {}

            std::optional<std::vector<std::chrono::sys_days>> get_additional_holidays(std::string_view code) const override 
            { 
                return chrono::get_additional_holidays(obj_, code); 
            }
            std::optional<std::vector<std::chrono::sys_days>> get_additional_businessdays(std::string_view code) const override 
            { 
                return chrono::get_additional_businessdays(obj_, code); 
            }
            std::type_index type() const noexcept override { return typeid(C); }
            const void* pointer() const noexcept override { return std::addressof(obj_); }

            C obj_;
        };

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        any_calendar_source() = delete;
        any_calendar_source(const this_type&) noexcept = default;
        any_calendar_source(this_type&&) noexcept = default;

        template <cpt::calendar_source Srv>
            requires cpt::different_from<Srv, this_type>
        any_calendar_source(Srv&& source)
            : obj_(std::make_shared<concrete<std::remove_cvref_t<Srv>>>(std::forward<Srv>(source)))
        {
        }

    // -------------------------------------------------------------------------
    //  calendar_source behavior
    //
        std::optional<std::vector<std::chrono::sys_days>> get_additional_holidays(std::string_view code) const { return obj_->get_additional_holidays(code); }
        std::optional<std::vector<std::chrono::sys_days>> get_additional_businessdays(std::string_view code) const { return obj_->get_additional_businessdays(code); }
        
    // -------------------------------------------------------------------------
    //  type_erasure behavior
    //
        std::type_index type() const noexcept { return obj_->type(); }

        template <cpt::calendar_source CS>
        util::maybe<const CS&> as() const noexcept
        {
            if (this->type() == typeid(CS)) {
                return util::maybe<const CS&>(*reinterpret_cast<const CS*>(obj_->pointer()));
            }
            else {
                return util::maybe<const CS&>();
            }
        }

    private:
        std::shared_ptr<const base> obj_;

    }; // class any_calendar_source
    
} // namespace egret::chrono
