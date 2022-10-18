#include <format>
#include <stacktrace>
#include <optional>

#include "../exception.h"

namespace {
    std::string _to_string(const std::stacktrace_entry& entry)
    {
        if (entry) {
            return std::format(
                "{}({}): {}",
                entry.source_file(),
                entry.source_line(),
                entry.description()
            );
        }
        else {
            return "empty std::stacktrace_entry";
        }
    }

} // namespace 

namespace egret {
// -----------------------------------------------------------------------------
//  [struct] exception::impl
// -----------------------------------------------------------------------------
    struct exception::impl {
        std::optional<std::string> location_description;
        std::optional<std::string> message;
    };

// -----------------------------------------------------------------------------
//  [class] exception
// -----------------------------------------------------------------------------
    exception::exception() noexcept
        : exception(1)
    {
    }

    exception::exception(int stacktrace_skip) noexcept
        : loc_state_(loc_state::not_constructed), 
          msg_state_(msg_state::not_constructed), 
          impl_(nullptr),
          stacktrace_(nullptr)
    {
        this->initialize();
        if (impl_ == nullptr) {
            return;
        }

        this->set_location_description(stacktrace_skip + 1);
    }


    const char* exception::what() const noexcept
    {
        if (impl_ == nullptr) {
            return "Error message is empty because initializing egret::exception object fails";
        }

        switch (msg_state_) {
        case egret::exception::msg_state::not_constructed:
            return "";

        case egret::exception::msg_state::format_err:
            return impl_->message
                ? impl_->message->c_str()
                : "Some std::format_error occurs during construction of egret::exception";

        case egret::exception::msg_state::bad_alloc:
            return impl_->message
                ? impl_->message->c_str()
                : "Some std::bad_alloc occurs during construction of egret::exception";

        case egret::exception::msg_state::unexpected_err:
            return impl_->message
                ? impl_->message->c_str()
                : "Some unexpected exception occurs during construction of egret::exception";

        case egret::exception::msg_state::well_constructed:
            return impl_->message
                ? impl_->message->c_str()
                : "";

        default:
            return 
                "Implementation error because control reaches unreachable 'default' of "
                "switch statement in egret::exception::what()";
        }
    }

    std::string_view exception::location_description() const noexcept
    {
        if (impl_ == nullptr) {
            return "Location description is empty because initializing egret::exception object fails";
        }

        switch (loc_state_) {
        case egret::exception::loc_state::not_constructed:
            return "Location description is not initialized";

        case egret::exception::loc_state::bad_alloc:
            return "Fail to initialize location description because of std::bad_alloc";
 
        case egret::exception::loc_state::unexpected_err:
            return "Fail to initialize location description because of unexpected error";

        case egret::exception::loc_state::well_constructed:
            return *(impl_->location_description);

        default:
            return 
                "Implementation error because control reaches unreachable 'default' of "
                "switch statement in egret::exception::location_description()";
        }
    }

    const std::vector<std::string>& exception::stacktrace() const noexcept
    {
        static const std::vector<std::string> empty;
        return stacktrace_ ? *stacktrace_ : empty;
    }

    exception& exception::record_stacktrace(int stacktrace_skip, int depth) & noexcept
    {
        // if stacktrace_ is shared with others, i.e. use_count() != 1,
        // create a new object to keep others invariant. (copy-on-write)
        if (stacktrace_.use_count() != 1) {
            std::shared_ptr<std::vector<std::string>> new_ptr = nullptr;
            try {
                new_ptr = stacktrace_.use_count() == 0
                    ? std::make_shared<std::vector<std::string>>()
                    : std::make_shared<std::vector<std::string>>(*stacktrace_);
            }
            catch (...) {
            }
            stacktrace_.swap(new_ptr);
        }

        if (stacktrace_ == nullptr) {
            return *this;
        }
        stacktrace_->clear();
        if (depth <= 0) {
            return *this;
        }
        try {
            // +1 is for this function call
            const auto st = std::stacktrace::current(
                static_cast<std::size_t>(std::max(0, stacktrace_skip)) + 1, 
                static_cast<unsigned>(depth)
            );
            stacktrace_->reserve(st.size());
            std::size_t i = 0;
            for (const auto& entry : st) {
                stacktrace_->push_back(std::to_string(i) + "> " + _to_string(entry));
                ++i;
            }            
        }
        catch (...) {
        }
        return *this;
    }

    exception& exception::record_stacktrace(int stacktrace_skip) & noexcept
    {
        return this->record_stacktrace(stacktrace_skip + 1, default_stacktrace_depth_);
    }

    exception& exception::record_stacktrace() & noexcept
    {
        return this->record_stacktrace(1, default_stacktrace_depth_);
    }

    exception&& exception::record_stacktrace(int stacktrace_skip, int depth) && noexcept
    {
        return std::move(this->record_stacktrace(stacktrace_skip, depth));
    }

    exception&& exception::record_stacktrace(int stacktrace_skip) && noexcept
    {
        return std::move(this->record_stacktrace(stacktrace_skip));
    }

    exception&& exception::record_stacktrace() && noexcept
    {
        return std::move(this->record_stacktrace());
    }

    void exception::initialize() noexcept
    {
        loc_state_ = loc_state::not_constructed;
        msg_state_ = msg_state::not_constructed;
        impl_.reset();

        try {
            impl_ = std::make_shared<impl>();
            stacktrace_ = std::make_shared<std::vector<std::string>>();
        }
        catch (...) {
        }
    }

    void exception::set_message(std::string&& message) noexcept
    {
        if (impl_ == nullptr) {
            return;
        }
        impl_->message.reset();
        impl_->message.emplace(std::move(message));
    }

    void exception::set_location_description(int stacktrace_skip) noexcept
    {
        if (impl_ == nullptr) {
            loc_state_ = loc_state::not_constructed;
            return;
        }

        try {
            // +1 is for this function call
            // TODO: use allocator on stack instead of default allocator!
            const auto st = std::stacktrace::current(
                static_cast<std::size_t>(std::max(0, stacktrace_skip)) + 1, 
                1
            );
            if (st.empty() || !st.at(0)) {
                return;
            }
            impl_->location_description = _to_string(st.at(0));
            loc_state_ = loc_state::well_constructed;
        }
        catch (const std::bad_alloc&) {
            loc_state_ = loc_state::bad_alloc;
        }
        catch (...) {
            loc_state_ = loc_state::unexpected_err;
        }
    }

    void exception::message_initialization_error(const char* err_class, const char* reason) noexcept
    {
        if (impl_ == nullptr) {
            return;
        }

        try {
            impl_->message.emplace(std::format(
                "An exception ({}) is thrown during construction of egret::exception. "
                "[detail=\"{}\"]",
                err_class,
                reason
            ));
        }
        catch (...) {
        }
    }

} // namespace egret
