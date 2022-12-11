#pragma once

#include "core/auto_link.h"
#include <memory>
#include <map>
#include <mutex>
#include "calendar.h"
#include "calendar_identifier.h"
#include "any_calendar_source.h"

namespace egret::chrono {
// -----------------------------------------------------------------------------
//  [class] calendar_server
// -----------------------------------------------------------------------------
    class calendar_server {
    private:
        using this_type = calendar_server;
        struct impl;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        calendar_server() = delete;
        calendar_server(const this_type&) noexcept = default;
        calendar_server(this_type&&) noexcept = default;

        explicit calendar_server(any_calendar_source src);

        this_type& operator =(const this_type&) noexcept = default;
        this_type& operator =(this_type&&) noexcept = default;

    // -------------------------------------------------------------------------
    //  get
    //
        calendar get(const calendar_identifier& key) const;

    private:
        std::shared_ptr<impl> impl_;

    }; // class calendar_server

} // namespace egret::chrono
