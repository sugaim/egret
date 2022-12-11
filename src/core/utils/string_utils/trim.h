#pragma once

#include <ranges>
#include <type_traits>
#include <string_view>

namespace egret_detail::trm_impl {
// -----------------------------------------------------------------------------
//  default_trim_chars
// -----------------------------------------------------------------------------
    template <typename CharT> 
    inline constexpr const CharT default_trim_chars[] {
        static_cast<CharT>(' '),
        static_cast<CharT>('\n'),
        static_cast<CharT>('\t')
    };

} // namespace egret_detail::trm_impl

namespace egret::util {
// -----------------------------------------------------------------------------
//  [fn] trim
// -----------------------------------------------------------------------------
    /**
     * @brief [fn] return a borrow range which represents trimmed string
     * 
     * @tparam Str borrowed_range type convertible to std::basic_string_view
     * @param str borrowed_range convertible to std::basic_string_view
     * @param trim_chars Chars to be trimmed
     * @return trimmed std::basic_string_view whose data is shared from 'str' 
    */
    template <std::ranges::borrowed_range Str>
        requires 
            requires {
                typename std::remove_cvref_t<Str>::traits_type;
            } 
            && std::constructible_from<
                std::basic_string_view<
                    std::ranges::range_value_t<Str>, 
                    typename std::remove_cvref_t<Str>::traits_type
                >,
                Str
            >
    constexpr auto trim(
        Str&& str, 
        const std::ranges::range_value_t<Str>* trim_chars 
            = egret_detail::trm_impl::default_trim_chars<std::ranges::range_value_t<Str>>
    )
        -> std::basic_string_view<
            std::ranges::range_value_t<Str>,
            typename std::remove_cvref_t<Str>::traits_type
        >
    {
        using result_type = std::basic_string_view<
            std::ranges::range_value_t<Str>,
            typename std::remove_cvref_t<Str>::traits_type
        >;
        const result_type sv = std::forward<Str>(str);
        const auto fst_idx = sv.find_first_not_of(trim_chars);
        const auto lst_idx = sv.find_last_not_of(trim_chars);

        return fst_idx != result_type::npos
            ? sv.substr(fst_idx, lst_idx - fst_idx + 1)
            : result_type {};
    }

    /**
     * @brief [fn] return a borrow range which represents trimmed string
     * 
     * @tparam Str borrowed_range type convertible to std::basic_string_view
     * @param str borrowed_range convertible to std::basic_string_view
     * @param trim_chars Chars to be trimmed
     * @return trimmed std::basic_string_view whose data is shared from 'str' 
    */    
    template <std::ranges::borrowed_range Str>
        requires 
            (!requires {
                typename std::remove_cvref_t<Str>::traits_type;
            }) 
            && std::constructible_from<
                std::basic_string_view<std::ranges::range_value_t<Str>>,
                Str
            >
    constexpr auto trim(
        Str&& str, 
        const std::ranges::range_value_t<Str>* trim_chars 
            = egret_detail::trm_impl::default_trim_chars<std::ranges::range_value_t<Str>>
    )
        -> std::basic_string_view<std::ranges::range_value_t<Str>>
    {
        using result_type = std::basic_string_view<std::ranges::range_value_t<Str>>;
        const result_type sv (std::forward<Str>(str));
        return util::trim(sv, trim_chars);
    }

    /**
     * @brief [fn] return a borrowed range which represents trimmed string
     * 
     * @tparam CharT char type
     * @param str pointer pointing array of chars. str must end with null-char
     * @param trim_chars Chars to be trimmed
     * @return trimmed std::basic_string_view whose data is shared from 'str' 
    */
    template <typename CharT>
    constexpr auto trim(
        const CharT* str, 
        const CharT* trim_chars 
            = egret_detail::trm_impl::default_trim_chars<CharT>
    )
        -> std::basic_string_view<CharT>
    {
        using result_type = std::basic_string_view<CharT>;
        const result_type sv {str};
        return util::trim(sv, trim_chars);
    }

// -----------------------------------------------------------------------------
//  [fn] trimmed
// -----------------------------------------------------------------------------
    /**
     * @brief [fn] return a string object which represents trimmed string
     * 
     * @tparam Str borrowed_range type convertible to std::basic_string_view
     * @param str borrowed_range convertible to std::basic_string_view
     * @param trim_chars Chars to be trimmed
     * @return trimmed std::basic_string whose data is shared from 'str' 
    */
    template <std::ranges::range Str>
        requires 
            requires {
                typename std::remove_cvref_t<Str>::traits_type;
            } 
            && std::constructible_from<
                std::basic_string_view<
                    std::ranges::range_value_t<Str>, 
                    typename std::remove_cvref_t<Str>::traits_type
                >,
                Str
            >
    constexpr auto trimmed(
        Str&& str, 
        const std::ranges::range_value_t<Str>* trim_chars 
            = egret_detail::trm_impl::default_trim_chars<std::ranges::range_value_t<Str>>
    )
        -> std::basic_string<
            std::ranges::range_value_t<Str>,
            typename std::remove_cvref_t<Str>::traits_type
        >
    {
        using result_t = std::basic_string<
            std::ranges::range_value_t<Str>,
            typename std::remove_cvref_t<Str>::traits_type
        >;
        const auto trimmed_sv = util::trim(str, trim_chars);
        result_t result (trimmed_sv.begin(), trimmed_sv.end());
        return result;
    }

    /**
     * @brief [fn] return a string object which represents trimmed string
     * 
     * @tparam Str borrowed_range type convertible to std::basic_string_view
     * @param str borrowed_range convertible to std::basic_string_view
     * @param trim_chars Chars to be trimmed
     * @return trimmed std::basic_string whose data is shared from 'str' 
    */    
    template <std::ranges::range Str>
        requires 
            (!requires {
                typename std::remove_cvref_t<Str>::traits_type;
            }) 
            && std::constructible_from<
                std::basic_string_view<std::ranges::range_value_t<Str>>,
                Str
            >
    constexpr auto trimmed(
        Str&& str, 
        const std::ranges::range_value_t<Str>* trim_chars 
            = egret_detail::trm_impl::default_trim_chars<std::ranges::range_value_t<Str>>
    )
        -> std::basic_string<std::ranges::range_value_t<Str>>
    {
        using result_type = std::basic_string_view<std::ranges::range_value_t<Str>>;
        const result_type sv (std::forward<Str>(str));
        return util::trimmed(sv, trim_chars);
    }

    /**
     * @brief [fn] return a string object which represents trimmed string
     * 
     * @tparam CharT char type
     * @param str pointer pointing array of chars. str must end with null-char
     * @param trim_chars Chars to be trimmed
     * @return trimmed std::basic_string whose data is shared from 'str' 
    */
    template <typename CharT>
    constexpr auto trimmed(
        const CharT* str, 
        const CharT* trim_chars 
            = egret_detail::trm_impl::default_trim_chars<CharT>
    )
        -> std::basic_string<CharT>
    {
        using result_type = std::basic_string_view<CharT>;
        const result_type sv {str};
        return util::trimmed(sv, trim_chars);
    }
    
} // namespace egret::util
