#pragma once

#include <string_view>
#include "case_insensitive.h"
#include "trim.h"

namespace egret::util {
// -----------------------------------------------------------------------------
//  [fn] insensitive_strcmp
// -----------------------------------------------------------------------------
    template <std::ranges::input_range Str1, std::ranges::input_range Str2>
        requires 
            std::constructible_from<std::basic_string_view<std::ranges::range_value_t<Str1>>, Str1> &&
            std::constructible_from<std::basic_string_view<std::ranges::range_value_t<Str2>>, Str2> &&
            std::same_as<std::ranges::range_value_t<Str1>, std::ranges::range_value_t<Str2>>
    auto insensitive_strcmp(const Str1& str1, const Str2& str2)
    {
        using char_type = std::ranges::range_value_t<Str1>;
        using basic_sv_t = std::basic_string_view<char_type>;
        using sv_t = case_insensitive_basic_string_view<char_type>;

        basic_sv_t bsv1 (str1);
        basic_sv_t bsv2 (str2);
        const sv_t sv1 = util::trim(sv_t(bsv1.data(), bsv1.size()));
        const sv_t sv2 = util::trim(sv_t(bsv2.data(), bsv2.size()));
        return sv1 <=> sv2;
    }

} // namespace egret::util
