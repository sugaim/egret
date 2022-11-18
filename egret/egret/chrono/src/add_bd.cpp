#include "../adjustments/add_bd.h"
#include "../adjustments/next_bd.h"
#include "../adjustments/prev_bd.h"

namespace egret::chrono {
// -----------------------------------------------------------------------------
//  [class] add_bd
// -----------------------------------------------------------------------------
    std::chrono::sys_days add_bd::operator()(const std::chrono::sys_days& d) const
    {
        if (count_ == 0) {
            return d;
        }
        const auto prev = prev_bd(cal_);
        const auto next = next_bd(cal_);
        const bool is_neg = count_ < 0;
        const auto size = is_neg ? - count_ : count_;

        std::chrono::sys_days result = is_neg ? prev(d) : next(d);
        if (is_neg) {
            for (int i = 0; i != size; ++i) {
                result = prev(result);
            }
        }
        else {
            for (int i = 0; i != size; ++i) {
                result = next(result);
            }
        }
        return result;
    }

} // namespace egret::chrono