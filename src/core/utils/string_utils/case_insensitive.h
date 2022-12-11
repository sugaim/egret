#pragma once

#include <string_view>
#include <type_traits>

namespace egret::util {
// -----------------------------------------------------------------------------
//  [trait] case_insensitive_char_traits
// -----------------------------------------------------------------------------
    template <typename CharT>
    struct case_insensitive_char_traits : std::char_traits<CharT> {
    private:
        using this_type = case_insensitive_char_traits;

        static CharT to_lower(CharT ch)
        {   
            if constexpr (std::is_same_v<CharT, char>) {
                return static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
            }
            else {
                const unsigned char uc = static_cast<unsigned char>(ch);
                const bool has_unsigned_char = ch != CharT{} && ch == static_cast<CharT>(uc);
                return has_unsigned_char ? static_cast<CharT>(std::tolower(uc)) : ch;
            }
        }

    public:
        using comparison_category = std::weak_ordering;

        static bool eq(CharT ch1, CharT ch2)
        {
            return this_type::to_lower(ch1) == this_type::to_lower(ch2);
        }

        static bool lt(CharT ch1, CharT ch2)
        {
            return this_type::to_lower(ch1) < this_type::to_lower(ch2);
        }

        static int compare(const CharT* s1, const CharT* s2, std::size_t n) {
            while ( n-- != 0 ) {
                if (this_type::to_lower(*s1) < this_type::to_lower(*s2)) return -1;
                if (this_type::to_lower(*s1) > this_type::to_lower(*s2)) return 1;
                ++s1; 
                ++s2;
            }
            return 0;
        }
        
        static const CharT* find(const CharT* s, std::size_t n, CharT a) {
            auto const ua (this_type::to_lower(a));
            while (n-- != 0) 
            {
                if (this_type::to_lower(*s) == ua) return s;
                s++;
            }
            return nullptr;
        }
    };

// -----------------------------------------------------------------------------
//  [type] case_insensitive_basic_string_view
// -----------------------------------------------------------------------------
    template <typename Char>
    using case_insensitive_basic_string_view = std::basic_string_view<Char, case_insensitive_char_traits<Char>>;

} // namespace egret::util
