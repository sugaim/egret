#include "../cashflows/ois_coupon_cf.h"

namespace egret::inst {
// -----------------------------------------------------------------------------
//  [fn] from_dto
//  [fn] to_dto
// -----------------------------------------------------------------------------
    ois_observation_convention<chrono::calendar> from_dto(
        const ois_observation_convention<chrono::calendar_identifier>& cnv,
        const chrono::calendar_server& server
    )
    {
        return {
            .lockout = cnv.lockout,
            .lookback = cnv.lookback,
            .observation_period_shift = cnv.observation_period_shift,
            .rate_reference_calendar = server.get(cnv.rate_reference_calendar),
            .compound_weight_calendar = server.get(cnv.compound_weight_calendar),
        };
    }

    ois_observation_convention<chrono::calendar_identifier> to_dto(
        const ois_observation_convention<chrono::calendar>& cnv
    )
    {
        return {
            .lockout = cnv.lockout,
            .lookback = cnv.lookback,
            .observation_period_shift = cnv.observation_period_shift,
            .rate_reference_calendar = cnv.rate_reference_calendar.identifier(),
            .compound_weight_calendar = cnv.compound_weight_calendar.identifier(),
        };
    }

} // namespace egret::inst
