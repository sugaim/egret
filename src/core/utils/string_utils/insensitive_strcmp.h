#pragma once

#include <string_view>
#include "case_insensitive.h"
#include "trim.h"

namespace egret::util {
// -----------------------------------------------------------------------------
//  [fn] insensitive_strcmp
// -----------------------------------------------------------------------------
    template <typename CharT = char, typename Str1, typename Str2>
        requires 
            std::constructible_from<std::basic_string_view<CharT>, Str1> &&
            std::constructible_from<std::basic_string_view<CharT>, Str2>
    std::weak_ordering insensitive_strcmp(const Str1& str1, const Str2& str2)
    {
        using sv_t = case_insensitive_basic_string_view<CharT>;
        std::basic_string_view<CharT> bsv1 (str1);
        std::basic_string_view<CharT> bsv2 (str2);
        const sv_t sv1 = util::trim(sv_t(bsv1.data(), bsv1.size()));
        const sv_t sv2 = util::trim(sv_t(bsv2.data(), bsv2.size()));
        return sv1 <=> sv2;
    }

} // namespace egret::util
