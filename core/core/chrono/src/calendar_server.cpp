#include <thread>
#include <ranges>
#include <numeric>
#include "../calendars/calendar_server.h"

namespace egret::chrono {
    namespace {
        // single calendar
        calendar get_calendar(const any_calendar_source& src, std::map<std::string, calendar>& map, const std::string& code)
        {
            if (map.contains(code)) {
                return map.at(code);
            }

            auto add_hols = [&src, &code] {
                auto maybe_add_hols = src.get_additional_holidays(code);
                return maybe_add_hols 
                    ? *std::move(maybe_add_hols)
                    : throw exception("Calendar source does not have additional holiday data for \"{}\"", code);
            }();
            auto add_bds = [&src, &code] {
                auto maybe_add_bds = src.get_additional_businessdays(code);
                return maybe_add_bds
                    ? *std::move(maybe_add_bds)
                    : throw exception("Calendar source does not have additional business day data for \"{}\"", code);
            }();

            // create result, register into cache and return it
            auto result = calendar({.codes = {code}},  std::move(add_hols), std::move(add_bds));
            map.emplace(code, result);
            return result;
        }

        // combined calendar
        calendar get_calendar(
            const any_calendar_source& src,
            std::map<std::string, calendar>& single_map,
            std::map<calendar_identifier, calendar>& map, 
            const calendar_identifier& key
        )
        {
            if (map.contains(key)) {
                return map.at(key);
            }

            const auto calendars = key.codes 
                | std::views::transform([&src, &single_map](const auto& code) { 
                    return get_calendar(src, single_map, code); 
                })
                | std::ranges::to<std::vector>();

            std::vector<std::chrono::sys_days> add_hols;
            std::vector<std::chrono::sys_days> add_bds;
            std::vector<std::chrono::sys_days> prev_add_hols {};
            std::vector<std::chrono::sys_days> prev_add_bds {};

            // reserve memory
            {
                const auto hol_cals_szs = calendars | std::views::transform([](const auto& cal) { return cal.additional_holidays().size(); });
                const auto bd_cals_szs  = calendars | std::views::transform([](const auto& cal) { return cal.additional_businessdays().size(); });

                switch (key.combination) {
                case calendar_combination::all_open:
                    add_hols.reserve(std::accumulate(hol_cals_szs.begin(), hol_cals_szs.end(), static_cast<std::size_t>(0)));
                    add_bds.reserve(std::ranges::max(bd_cals_szs));
                    break;
                case calendar_combination::any_open:
                    add_hols.reserve(std::ranges::max(hol_cals_szs));
                    add_bds.reserve(std::accumulate(bd_cals_szs.begin(), bd_cals_szs.end(), static_cast<std::size_t>(0)));
                    break;
                default:
                    break;
                }
                prev_add_hols.reserve(add_hols.capacity());
                prev_add_bds.reserve(add_bds.capacity());
            }

            // intersect/union with remained calendars
            for (const auto& cal : calendars) {
                add_hols.clear();
                add_bds.clear();
                switch (key.combination) {
                case calendar_combination::all_open:
                    std::ranges::set_union(prev_add_hols, cal.additional_holidays(), std::back_inserter(add_hols));
                    std::ranges::set_intersection(prev_add_bds, cal.additional_businessdays(), std::back_inserter(add_bds));
                    break;
                case calendar_combination::any_open:
                    std::ranges::set_intersection(prev_add_hols, cal.additional_holidays(), std::back_inserter(add_hols));
                    std::ranges::set_union(prev_add_bds, cal.additional_businessdays(), std::back_inserter(add_bds));
                    break;
                default:
                    break;
                }
                prev_add_hols = add_hols;
                prev_add_bds = add_bds;
            }

            auto result = calendar(key, std::move(add_hols), std::move(add_bds));
            map.emplace(key, result);
            return result;
        }

    } // namespace 

// -----------------------------------------------------------------------------
//  [class] calendar_server
// -----------------------------------------------------------------------------
    struct calendar_server::impl {
        const any_calendar_source src;
        std::map<std::string, calendar> single_cache;
        std::map<calendar_identifier, calendar> cache;
        std::mutex mutex;
    };

    calendar_server::calendar_server(any_calendar_source src)
        : impl_(std::make_shared<impl>(std::move(src)))
    {
    }

    calendar calendar_server::get(const calendar_identifier& key) const
    {
        const auto _ = std::lock_guard<std::mutex>(impl_->mutex);

        switch (key.codes.size()) {
        case 0:
            return calendar(key, {}, {});
        case 1:
            return get_calendar(impl_->src, impl_->single_cache, (*key.codes.begin()));        
        default:
            return get_calendar(impl_->src, impl_->single_cache, impl_->cache, key);
        }
    }

} // namespace egret::chrono