#pragma once

#include "core/auto_link.h"
#include <utility>
#include <stdexcept>
#include <string_view>
#include <string>
#include <vector>
#include <format>
#include <memory>

namespace egret {
// -----------------------------------------------------------------------------
//  [class] exception
// -----------------------------------------------------------------------------
    class exception : public std::exception {
    private:
        using this_type = exception;

        struct impl;
        enum class loc_state : char {
            not_constructed,
            well_constructed,
            bad_alloc,
            unexpected_err,
        };
        enum class msg_state : char {
            not_constructed,
            well_constructed,
            format_err,
            bad_alloc,
            unexpected_err,
        };

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtors and assignments
    //
        exception() noexcept;
        exception(const this_type&) noexcept = default;
        exception(this_type&&) noexcept = default;

        explicit exception(int stacktrace_skip) noexcept;
        
        template <typename ...Args>
        explicit exception(int stacktrace_skip, std::string_view fmt, Args&& ...args) noexcept
            : exception(stacktrace_skip + 1)
        {
            if (impl_ == nullptr) {
                return;
            }

            try {
                std::string msg = std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...));
                this->set_message(std::move(msg));
                msg_state_ = msg_state::well_constructed;
            }
            catch (const std::format_error& e) {
                this->message_initialization_error("std::format_error", e.what());
                msg_state_ = msg_state::format_err;
            }
            catch (const std::bad_alloc& e) {
                this->message_initialization_error("std::bad_alloc", e.what());
                msg_state_ = msg_state::bad_alloc;
            }
            catch (const std::exception& e) {
                this->message_initialization_error("std::exception", e.what());
                msg_state_ = msg_state::unexpected_err;
            }
            catch (...) {
                msg_state_ = msg_state::unexpected_err;
            }
        }

        template <typename ...Args>
        explicit exception(std::string_view fmt, Args&& ...args) noexcept
            : exception(1, fmt, std::forward<Args>(args)...)
        {
        }

        this_type& operator =(const this_type&) noexcept = default;
        this_type& operator =(this_type&&) noexcept = default;

    // -------------------------------------------------------------------------
    //  get  
    //
        const char* what() const noexcept override;
        std::string_view location_description() const noexcept;
        const std::vector<std::string>& stacktrace() const noexcept;

        static int& default_stacktrace_depth() noexcept { return default_stacktrace_depth_; }

    // -------------------------------------------------------------------------
    //  set
    //
        this_type&& record_stacktrace(int stacktrace_skip, int depth) && noexcept;
        this_type&& record_stacktrace(int stacktrace_skip) && noexcept;
        this_type&& record_stacktrace() && noexcept;

        this_type& record_stacktrace(int stacktrace_skip, int depth) & noexcept;
        this_type& record_stacktrace(int stacktrace_skip) & noexcept;
        this_type& record_stacktrace() & noexcept;

    private:
        inline static int default_stacktrace_depth_ = 10;

        void initialize() noexcept;
        void set_message(std::string&& message) noexcept;
        void set_location_description(int stacktrace_skip) noexcept;
        void message_initialization_error(const char* err_class, const char* reason) noexcept;

    private:
        loc_state loc_state_ = loc_state::not_constructed;
        msg_state msg_state_ = msg_state::not_constructed;
        std::shared_ptr<impl> impl_;
        std::shared_ptr<std::vector<std::string>> stacktrace_;

    }; // class exception

} // namespace egret
