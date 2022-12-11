#include <regex>
#include "core/assertions/exception.h"
#include "core/utils/string_utils/trim.h"
#include "../tenor.h"

namespace egret::chrono {
// -----------------------------------------------------------------------------
//  [class] tenor
// -----------------------------------------------------------------------------
    tenor tenor::parse(std::string_view sv)
    {
        static const std::regex D("^-?[0-9]+[ ]*D$", std::regex_constants::icase);
        static const std::regex days("^-?[0-9]+[ ]*days$", std::regex_constants::icase);
        static const std::regex W("^-?[0-9]+[ ]*W$", std::regex_constants::icase);
        static const std::regex weeks("^-?[0-9]+[ ]*weeks$", std::regex_constants::icase);
        static const std::regex M("^-?[0-9]+[ ]*M$", std::regex_constants::icase);
        static const std::regex months("^-?[0-9]+[ ]*months$", std::regex_constants::icase);
        static const std::regex Y("^-?[0-9]+[ ]*Y$", std::regex_constants::icase);
        static const std::regex years("^-?[0-9]+[ ]*years$", std::regex_constants::icase);

        sv = util::trim(sv);
        const auto match = [sv] (const std::regex& ptn) -> bool {
            return std::regex_match(sv.begin(), sv.end(), ptn);
        };
        tenor_unit unit = tenor_unit::days;

        if (match(D) || match(days)) {
            unit = tenor_unit::days;
        }
        else if (match(W) || match(weeks)) {
            unit = tenor_unit::weeks;
        }
        else if (match(M) || match(months)) {
            unit = tenor_unit::months;
        }
        else if (match(Y) || match(years)) {
            unit = tenor_unit::years;
        }
        else {
            throw exception(
                "\"{}\" is in unsupported tenor format. "
                "Only \"^-?[0-9]+[ ]*(D|days|W|weeks|M|months|Y|years)$\" is supported. "
                "(case insensitive, start/end whitespaces are ignored)", sv).record_stacktrace();
        }

        std::int32_t count = 0;
        const auto [end, err] = std::from_chars(sv.data(), sv.data() + sv.size(), count);
        if (err != std::errc()) {
            throw exception("Fail to parse integer part of tenor string \"{}\"", sv).record_stacktrace();
        }
        return tenor(count, unit);
    }

    std::chrono::sys_days operator +(const std::chrono::sys_days& lhs, const tenor& tnr)
    {
        switch (tnr.unit()) {
        case tenor_unit::days:
            return lhs + std::chrono::days(tnr.count());
        case tenor_unit::weeks:
            return lhs + std::chrono::days(tnr.count() * 7);
        case tenor_unit::months:
            return std::chrono::year_month_day(lhs) + std::chrono::months(tnr.count());
        case tenor_unit::years:
            return std::chrono::year_month_day(lhs) + std::chrono::years(tnr.count());            
        default:
            throw exception("Unexpected tenor unit");
        }
    }

    std::chrono::sys_days operator -(const std::chrono::sys_days& lhs, const tenor& tnr)
    {
        return lhs + (-tnr);
    }

    std::chrono::sys_seconds operator +(const std::chrono::sys_seconds& lhs, const tenor& tnr)
    {
        const auto date = std::chrono::time_point_cast<std::chrono::days>(lhs);
        const auto secs = lhs - date;
        return (date + tnr) + secs;
    }

    std::chrono::sys_seconds operator -(const std::chrono::sys_seconds& lhs, const tenor& tnr)
    {
        return lhs + (-tnr);
    }

} // namespace egret::chrono
