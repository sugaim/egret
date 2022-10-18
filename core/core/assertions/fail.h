#pragma once

#include <utility>
#include <string_view>
#include "exception.h"

namespace egret {
// -----------------------------------------------------------------------------
//  [fn] fail
// -----------------------------------------------------------------------------
    template <typename ...Args>
    void fail(std::string_view fmt, Args&& ...args)
    {
        throw exception(fmt, std::forward<Args>(args)...).record_stacktrace();
    }

} // namespace egret
