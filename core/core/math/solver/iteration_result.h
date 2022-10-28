#pragma once

#include <cstddef>
#include <list>
#include <optional>
#include <expected>
#include "core/assertions/exception.h"

namespace egret::math::solver {
// -----------------------------------------------------------------------------
//  [struct] iteration_log_item
// -----------------------------------------------------------------------------
    template <typename LogValue>
    struct iteration_log_item {
        std::size_t iteration;
        LogValue log;
    };

// -----------------------------------------------------------------------------
//  [class] iteration_result
// -----------------------------------------------------------------------------
    template <typename X, typename LogValue = X>
    class iteration_result {
    private:
        using this_type = iteration_result;

    public:
    // -------------------------------------------------------------------------
    //  set
    //
        void set_error(exception exp) noexcept { result_ = std::expected<X, exception>(std::unexpect, std::move(exp)); }
        void set_result(X value) { result_.emplace(std::move(value)); }
        void push(std::size_t i, LogValue value) { log_.emplace_back(i, std::move(value)); }

    // -------------------------------------------------------------------------
    //  get
    //
        const std::expected<X, exception>& result() const noexcept { return result_; }
        const std::list<iteration_log_item<LogValue>>& log() const noexcept { return log_; }

    private:
        std::expected<X, exception> result_ = std::expected<X, exception>(std::unexpect, "result for iteration_result is not set yet.");
        std::list<iteration_log_item<LogValue>> log_;

    }; // class iteration_result

} // namespace egret::math::solver
