#include "core/utils/string_utils/trim.h"

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
//  evaluate_impl
// -----------------------------------------------------------------------------
    template <typename TestType, typename Trimmer>
    auto evaluate_impl(const Trimmer& trimmer, const std::string_view expected, const std::string_view input)
    {
        constexpr auto converter = cstring_to_test_type<char, TestType> {};
        const auto escaped = escape_escaped(input);
        const auto converted = converter(input.data());
        const auto actual = trimmer(converted);
        return actual == expected
            ? ::testing::AssertionSuccess()
            : ::testing::AssertionFailure() 
                << "expected=\"" << expected << "\" "
                << "but actual=\"" << actual << "\" "
                << "for input \"" << escaped << "\"";
    }

// -----------------------------------------------------------------------------
//  test_impl
// -----------------------------------------------------------------------------
    template <typename TestType, typename Cases>
    void test_impl(const std::string_view expected, const Cases& cases)
    {
        constexpr auto is_trim_result_expected = [&](const char* param) {
            return egret::tests::evaluate_impl<TestType>(
                [](const auto& str) { return egret::util::trim(str); }, 
                expected, param
            );
        };
        constexpr auto is_trimmed_result_expected = [&](const char* param) {
            return egret::tests::evaluate_impl<TestType>(
                [](const auto& str) { return egret::util::trimmed(str); }, 
                expected, param
            );    
        };
        for (const auto& case_input : cases) {
            EXPECT_TRUE(is_trim_result_expected(case_input));
            EXPECT_TRUE(is_trimmed_result_expected(case_input));
        }
    }

}} // namespace egret::tests

template <typename T>
class trim : public ::testing::Test {};

TYPED_TEST_CASE_P(trim);

TYPED_TEST_P(trim, empty) {
    constexpr std::string_view expected = "";
    constexpr auto cases = std::array {
        "",
        " ",
        "\n",
        "\t",
        "\n\t",
        " \n \t ",
    };

    egret::tests::test_impl<TypeParam>(expected, cases);
}

TYPED_TEST_P(trim, trim_right) {
    constexpr std::string_view expected = "hoge";
    constexpr auto cases = std::array {
        "hoge ",
        "hoge  ",
        "hoge\n",
        "hoge\t",
        "hoge\t \n",
    };

    egret::tests::test_impl<TypeParam>(expected, cases);
}
TYPED_TEST_P(trim, separated_trim_right) {
    constexpr std::string_view expected = "ho ge";
    constexpr auto cases = std::array {
        "ho ge ",
        "ho ge  ",
        "ho ge\n",
        "ho ge\t",
        "ho ge\t \n",
    };

    egret::tests::test_impl<TypeParam>(expected, cases);
}

TYPED_TEST_P(trim, trim_left) {
    constexpr std::string_view expected = "hoge";
    constexpr auto cases = std::array {
        " hoge",
        "  hoge",
        "\nhoge",
        "\thoge",
        "\t \nhoge",
    };

    egret::tests::test_impl<TypeParam>(expected, cases);
}
TYPED_TEST_P(trim, separated_trim_left) {
    constexpr std::string_view expected = "ho ge";
    constexpr auto cases = std::array {
        " ho ge",
        "  ho ge",
        "\nho ge",
        "\tho ge",
        "\t \nho ge",
    };

    egret::tests::test_impl<TypeParam>(expected, cases);
}
TYPED_TEST_P(trim, trim_both) {
    constexpr std::string_view expected = "hoge";
    constexpr auto cases = std::array {
        "hoge",
        "  hoge  ",
        "\nhoge\n",
        "\nhoge\t",
        " hoge\n",
        " hoge\n\t",
        " hoge\n \t",
        "\thoge\n \t",
        "\n \nhoge\n \t",
        "\nhoge ",
        "\thoge ",
        "\n\thoge ",
        "\n \thoge ",
        "\n \thoge\t",
        "\n \thoge\t \n",
    };

    egret::tests::test_impl<TypeParam>(expected, cases);
}
TYPED_TEST_P(trim, separated_trim_both) {
    constexpr std::string_view expected = "ho ge";
    constexpr auto cases = std::array {
        "ho ge",
        "  ho ge  ",
        "\nho ge\n",
        "\nho ge\t",
        " ho ge\n",
        " ho ge\n\t",
        " ho ge\n \t",
        "\tho ge\n \t",
        "\n \nho ge\n \t",
        "\nho ge ",
        "\tho ge ",
        "\n\tho ge ",
        "\n \tho ge ",
        "\n \tho ge\t",
        "\n \tho ge\t \n",
    };

    egret::tests::test_impl<TypeParam>(expected, cases);
}

REGISTER_TYPED_TEST_CASE_P(trim,
    empty, 
    trim_right, trim_left, trim_both,
    separated_trim_right, separated_trim_left, separated_trim_both);

// instantinate
using char_base_string_types = ::testing::Types<
    std::string, std::string_view, const char*,
    std::vector<char>>;
INSTANTIATE_TYPED_TEST_CASE_P(StringUtil, trim, char_base_string_types);
