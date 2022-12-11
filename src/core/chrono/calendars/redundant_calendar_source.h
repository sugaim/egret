#pragma once

#include <vector>
#include <iterator>
#include <chrono>
#include <type_traits>
#include "concepts.h"

namespace egret::chrono {
// -----------------------------------------------------------------------------
//  [class] redundant_calendar_source
// -----------------------------------------------------------------------------
    template <cpt::calendar_source Src>
    class redundant_calendar_source {
    private:
        using this_type = redundant_calendar_source;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        redundant_calendar_source() = default;
        redundant_calendar_source(const this_type&) = default;
        redundant_calendar_source(this_type&&)
            noexcept(std::is_nothrow_move_constructible_v<std::vector<Src>>) = default;

        redundant_calendar_source(std::vector<Src> sources)
            noexcept(std::is_nothrow_move_constructible_v<std::vector<Src>>)
            : sources_(std::move(sources))
        {
        }

    // -------------------------------------------------------------------------
    //  calendar_source behavior
    //
        std::optional<std::vector<std::chrono::sys_days>> get_additional_holidays(std::string_view code) const
        {
            for (const auto& source : sources_) {
                if (auto found = chrono::get_additional_holidays(source, code)) {
                    return found;
                }
            }
            return std::nullopt;
        }
        std::optional<std::vector<std::chrono::sys_days>> get_additional_businessdays(std::string_view code) const
        {
            for (const auto& source : sources_) {
                if (auto found = chrono::get_additional_businessdays(source, code)) {
                    return found;
                }
            }
            return std::nullopt;
        }

    private:
        std::vector<Src> sources_;
        
    }; // class redundant_calendar_source

} // namespace egret::chrono
