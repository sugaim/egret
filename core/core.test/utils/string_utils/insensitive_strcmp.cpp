#include "core/utils/string_utils/insensitive_strcmp.h"

namespace egret::tests { namespace {
// -----------------------------------------------------------------------------
//  converter
// -----------------------------------------------------------------------------
    template <typename CharT, typename T>
    struct cstring_to_test_type {};

    template <typename CharT>
    struct cstring_to_test_type<CharT, const CharT*> {
        const CharT* operator()(const CharT* data) const noexcept 
        { 
            return data; 
        }
    };

    template <typename CharT>
    struct cstring_to_test_type<CharT, std::basic_string<CharT>> {
        std::basic_string<CharT> operator()(const CharT* data) const 
        { 
            return std::basic_string<CharT>(data); 
        }
    };

    template <typename CharT>
    struct cstring_to_test_type<CharT, std::basic_string_view<CharT>> {
        std::basic_string_view<CharT> operator()(const CharT* data) const 
        { 
            return std::basic_string_view<CharT>(data); 
        }
    };

    template <typename CharT>
    struct cstring_to_test_type<CharT, std::vector<CharT>> {
        std::vector<CharT> operator()(const CharT* data) const 
        { 
            const std::size_t size = std::char_traits<CharT>::length(data);
            std::vector<CharT> result {};
            result.resize(size);
            for (std::size_t i = 0; i < size; ++i) {
                result[i] = data[i];
            }
            return result; 
        }
    };

// -----------------------------------------------------------------------------
//  escape_escaped
// -----------------------------------------------------------------------------
    std::string escape_escaped(const std::string_view sv)
    {
        std::string result;
        for (const char ch : sv) {
            switch (ch)
            {
            case '\"':
                result += "\\\"";
                break;                
            case '\n':
                result += "\\n";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                result += ch;
            }
        }
        return result;
    }

// -----------------------------------------------------------------------------
//  is_pair
// -----------------------------------------------------------------------------
    template <typename T>
    inline constexpr bool is_pair = false;

    template <typename T1, typename T2>
    inline constexpr bool is_pair<std::pair<T1, T2>> = true;

// -----------------------------------------------------------------------------
//  case_t
// -----------------------------------------------------------------------------
    template <typename CharT>
    struct case_t {
        std::basic_string_view<CharT> lhs;
        std::basic_string_view<CharT> rhs;
        std::weak_ordering expected;
    };

// -----------------------------------------------------------------------------
//  test_impl
// -----------------------------------------------------------------------------
    std::string_view to_string(const std::weak_ordering& ord)
    {
        if (ord == std::weak_ordering::less) return "less";
        else if (ord == std::weak_ordering::greater) return "greater";
        else return "equivalent";
    }
    std::weak_ordering flip(const std::weak_ordering& ord)
    {
        if (ord == std::weak_ordering::less) return std::weak_ordering::greater;
        else if (ord == std::weak_ordering::greater) return std::weak_ordering::less;
        else return std::weak_ordering::equivalent;
    }
    
    template <typename LType, typename RType>
    auto evaluate_impl(const case_t<char>& input)
    {
        constexpr auto lhs_converter = cstring_to_test_type<char, LType> {};
        constexpr auto rhs_converter = cstring_to_test_type<char, RType> {};
        const auto lhs = lhs_converter(input.lhs.data());
        const auto rhs = rhs_converter(input.rhs.data());
        const auto actual = egret::util::insensitive_strcmp(lhs, rhs);
        return actual == input.expected
            ? ::testing::AssertionSuccess()
            : ::testing::AssertionFailure() 
                << "expected=\"" << to_string(input.expected) << "\" "
                << "but actual=\"" << to_string(actual) << "\" "
                << "for lhs \"" << escape_escaped(input.lhs) << "\" "
                << "and rhs \"" << escape_escaped(input.rhs) << "\"";
    }
    
    template <typename LType, typename RType, typename CharT>
    auto test_impl(const std::vector<case_t<CharT>>& cases)
    {
        constexpr auto is_insensitive_strcmp_result_expected = [](const case_t<CharT>& c) {
            return evaluate_impl<LType, RType>(c);
        };
        constexpr auto is_flipped_insensitive_strcmp_result_expected = [](const case_t<CharT>& c) {
            case_t<CharT> flipped;
            flipped.lhs = c.rhs;
            flipped.rhs = c.lhs;
            flipped.expected = flip(c.expected);
            return evaluate_impl<RType, LType>(flipped);
        };
        for (const auto& case_input : cases) {
            EXPECT_TRUE(is_insensitive_strcmp_result_expected(case_input));
            EXPECT_TRUE(is_flipped_insensitive_strcmp_result_expected(case_input));
        }
    }
    
}} // namespace egret::tests

template <typename T>
    requires egret::tests::is_pair<T>
class insensitive_strcmp : public ::testing::Test {};

TYPED_TEST_CASE_P(insensitive_strcmp);

TYPED_TEST_P(insensitive_strcmp, case_insencitive_eq) {
    using case_t = egret::tests::case_t<char>;
    const auto equiv_strings1 = std::vector{
        "a", "A"
    };
    const auto equiv_strings2 = std::vector{
        "hoge", "HOGE", "hogE", "Hoge", "HoGe", "hOgE", "hOge"
    };
    const auto equiv_strings3 = std::vector{
        "hog e", "HOG E", "hog E", "Hog e", "HoG e", "hOg E", "hOg e"
    };
    auto cases = std::vector<case_t> {};
    for (const auto& s1 : equiv_strings1) {
        for (const auto& s2 : equiv_strings1) {
            cases.emplace_back(s1, s2, std::weak_ordering::equivalent);
        }
    }
    for (const auto& s1 : equiv_strings2) {
        for (const auto& s2 : equiv_strings2) {
            cases.emplace_back(s1, s2, std::weak_ordering::equivalent);
        }
    }
    for (const auto& s1 : equiv_strings3) {
        for (const auto& s2 : equiv_strings3) {
            cases.emplace_back(s1, s2, std::weak_ordering::equivalent);
        }
    }

    using lhs = TypeParam::first_type;
    using rhs = TypeParam::second_type;
    egret::tests::test_impl<lhs, rhs>(cases);
}

TYPED_TEST_P(insensitive_strcmp, trimmed_eq) {
    using case_t = egret::tests::case_t<char>;
    auto cases = std::vector<case_t> {};
    const auto equiv_strings1 = std::vector{
        "", " ", "\n", "\t", "\n \t", " \n\t\n "
    };
    const auto equiv_strings2 = std::vector{
        "a", 
        "a ", " a", " a ", "\na", "a\n", "\ta", "a\t",
        "a  ", "  a", " a ", "a\n\n", "\n\na", "\na\n", "a\t\t", "\t\ta", "\ta\t",
        "a \n", "a\n ", " a\n", "\na ", " \na", "\n a", 
        "a \t", "a\t ", " a\t", "\ta ", " \ta", "\t a", 
        "a\n\t", "a\t\n", "\na\t", "\ta\n", "\n\ta", "\t\na", 
        " \t  a\n\n   \t "
    };
    const auto equiv_strings3 = std::vector{
        "A B C", 
        "A B C ", " A B C", " A B C ", "\nA B C", "A B C\n", "\tA B C", "A B C\t",
        "A B C  ", "  A B C", " A B C ", "A B C\n\n", "\n\nA B C", "\nA B C\n", "A B C\t\t", "\t\tA B C", "\tA B C\t",
        "A B C \n", "A B C\n ", " A B C\n", "\nA B C ", " \nA B C", "\n A B C", 
        "A B C \t", "A B C\t ", " A B C\t", "\tA B C ", " \tA B C", "\t A B C", 
        "A B C\n\t", "A B C\t\n", "\nA B C\t", "\tA B C\n", "\n\tA B C", "\t\nA B C", 
        " \t  A B C\n\n   \t "
    };
    for (const auto& s1 : equiv_strings1) {
        for (const auto& s2 : equiv_strings1) {
            cases.emplace_back(s1, s2, std::weak_ordering::equivalent);
        }
    }
    for (const auto& s1 : equiv_strings2) {
        for (const auto& s2 : equiv_strings2) {
            cases.emplace_back(s1, s2, std::weak_ordering::equivalent);
        }
    }
    for (const auto& s1 : equiv_strings3) {
        for (const auto& s2 : equiv_strings3) {
            cases.emplace_back(s1, s2, std::weak_ordering::equivalent);
        }
    }

    using lhs = TypeParam::first_type;
    using rhs = TypeParam::second_type;
    egret::tests::test_impl<lhs, rhs>(cases);
}

TYPED_TEST_P(insensitive_strcmp, case_insensitive_trimmed_eq) {
    using case_t = egret::tests::case_t<char>;
    auto cases = std::vector<case_t> {};
    const auto equiv_strings = std::vector{
        "A B C", 
        "A B C ", " A B C", " A B C ", "\nA B C", "A B C\n", "\tA B C", "A B C\t",
        "A B C  ", "  A B C", " A B C ", "A B C\n\n", "\n\nA B C", "\nA B C\n", "A B C\t\t", "\t\tA B C", "\tA B C\t",
        "A B C \n", "A B C\n ", " A B C\n", "\nA B C ", " \nA B C", "\n A B C", 
        "A B C \t", "A B C\t ", " A B C\t", "\tA B C ", " \tA B C", "\t A B C", 
        "A B C\n\t", "A B C\t\n", "\nA B C\t", "\tA B C\n", "\n\tA B C", "\t\nA B C", 
        " \t  A B C\n\n   \t ",
        "a b c", 
        "a b c ", " a b c", " a b c ", "\na b c", "a b c\n", "\ta b c", "a b c\t",
        "a b c  ", "  a b c", " a b c ", "a b c\n\n", "\n\na b c", "\na b c\n", "a b c\t\t", "\t\ta b c", "\ta b c\t",
        "a b c \n", "a b c\n ", " a b c\n", "\na b c ", " \na b c", "\n a b c", 
        "a b c \t", "a b c\t ", " a b c\t", "\ta b c ", " \ta b c", "\t a b c", 
        "a b c\n\t", "a b c\t\n", "\na b c\t", "\ta b c\n", "\n\ta b c", "\t\na b c", 
        " \t  a b c\n\n   \t ",
        "a b C", 
        "a b C ", " a b C", " a b C ", "\na b C", "a b C\n", "\ta b C", "a b C\t",
        "a b C  ", "  a b C", " a b C ", "a b C\n\n", "\n\na b C", "\na b C\n", "a b C\t\t", "\t\ta b C", "\ta b C\t",
        "a b C \n", "a b C\n ", " a b C\n", "\na b C ", " \na b C", "\n a b C", 
        "a b C \t", "a b C\t ", " a b C\t", "\ta b C ", " \ta b C", "\t a b C", 
        "a b C\n\t", "a b C\t\n", "\na b C\t", "\ta b C\n", "\n\ta b C", "\t\na b C", 
        " \t  a b C\n\n   \t ",
    };
    for (const auto& s1 : equiv_strings) {
        for (const auto& s2 : equiv_strings) {
            cases.emplace_back(s1, s2, std::weak_ordering::equivalent);
        }
    }

    using lhs = TypeParam::first_type;
    using rhs = TypeParam::second_type;
    egret::tests::test_impl<lhs, rhs>(cases);
}

REGISTER_TYPED_TEST_CASE_P(insensitive_strcmp,
    case_insencitive_eq, trimmed_eq, case_insensitive_trimmed_eq);

// instantinate
using char_base_string_types = ::testing::Types<
    std::pair<std::string, std::string>, 
    std::pair<std::string_view, std::string_view>, 
    std::pair<const char*, const char*>,
    std::pair<std::vector<char>, std::vector<char>>,
    std::pair<std::string, std::string_view>,
    std::pair<std::string, const char*>,
    std::pair<std::string, std::vector<char>>,
    std::pair<std::string_view, const char*>,
    std::pair<std::string_view, std::vector<char>>,
    std::pair<const char*, std::vector<char>>>;
INSTANTIATE_TYPED_TEST_CASE_P(StringUtil, insensitive_strcmp, char_base_string_types);
