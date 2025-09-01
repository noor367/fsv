#include "./filtered_string_view.h"
#include <catch2/catch.hpp>
#include <iostream>

TEST_CASE("CONSTRUCTORS") {
    SECTION("default constructor initialises empty view") {
        auto s = fsv::filtered_string_view();
        CHECK(s.size() == 0);
    }

    auto const str = std::string{"cat"};
    SECTION("std::string constructor initialises correct values") {
        auto s = fsv::filtered_string_view{str};
        CHECK(s.size() == 3);
        auto ptr = s.data();
        auto result = std::string{};
        while (*ptr) {
            result += *ptr++;
        }
        CHECK(result == "cat");
    }

    auto pred = [](const char& c) { return c == 'a'; };
    SECTION("std::string constructor with predicate") {
        auto s = fsv::filtered_string_view{str, pred};
        CHECK(s.size() == 1);
        auto ptr = s.data();
        auto result = std::string{};
        while (*ptr) {
            result += *ptr++;
        }
        CHECK(result == "cat");
    }

    SECTION("null terminated constructor") {
        auto s = fsv::filtered_string_view{"cat"};
        CHECK(s.size() == 3);
        auto ptr = s.data();
        auto result = std::string{};
        while (*ptr) {
            result += *ptr++;
        }
        CHECK(result == "cat");
    }

    SECTION("null terminated constructor with predicate") {
        auto s = fsv::filtered_string_view{"cat", pred};
        CHECK(s.size() == 1);
        auto ptr = s.data();
        auto result = std::string{};
        while (*ptr) {
            result += *ptr++;
        }
        CHECK(result == "cat");
    }

    SECTION("copy constructor") {}

    SECTION("move constructor") {}
}

TEST_CASE("MEMBER FUNCTIONS") {
    SECTION("empty - false case") {
        auto s = fsv::filtered_string_view{"Australian Shephard"};
        CHECK(s.empty() == false);
    }
    SECTION("empty - true case") {
        auto s = fsv::filtered_string_view{};
        CHECK(s.empty() == true);
    }

    SECTION("data - spaces") {
        auto s = fsv::filtered_string_view{"Answer to life is 42", [](const char&) { return false; }};
        auto ptr = s.data();
        auto result = std::string{};
        while (*ptr) {
            result += *ptr++;
        }

        CHECK(result == "Answer to life is 42");
    }

    SECTION("data - special characters") {
        auto s = fsv::filtered_string_view{"@ngryb!rd5"};
        auto ptr = s.data();
        auto result = std::string{};
        while (*ptr) {
            result += *ptr++;
        }

        CHECK(result == "@ngryb!rd5");
    }

    SECTION("predicate - works") {
        const auto pred = [](const char&) {
            std::cout << "hi!";
            return true;
        };

        auto s = fsv::filtered_string_view{"doggo", pred};

        const auto& p = s.predicate();
        CHECK(p('a'));
    }
}

TEST_CASE("NON-MEMBER OPERATORS") {
    auto apples = fsv::filtered_string_view{"apples"};
    SECTION("equality - true") {
        CHECK(apples == apples);
    }
    auto ban = fsv::filtered_string_view{"bananas"};
    SECTION("equality - false") {
        CHECK(apples != ban);
    }
    auto fban = fsv::filtered_string_view{"bananas", [](char const& c) { return c != 'a' && c != ' '; }};
    auto bnns = fsv::filtered_string_view{"bnns"};
    SECTION("equality - filtered true") {
        CHECK(fban == bnns);
    }
    SECTION("equality - filtered false") {
        CHECK_FALSE(ban == fban);
        CHECK(ban != fban);
    }

    // TESTING >
    SECTION("gt - true") {
        CHECK(ban > apples);
    }
    SECTION("gt - false") {
        CHECK_FALSE(apples > ban);
    }
    auto lo = fsv::filtered_string_view{"horn"};
    auto hi = fsv::filtered_string_view{"a mule", [](char const& c) { return c != 'a' && c != ' '; }};
    SECTION("gt - filtered true") {
        CHECK(hi > lo);
    }
    SECTION("gt - filtered false") {
        CHECK_FALSE(lo > hi);
    }

    // TESTING >=
    SECTION("gte - true") {
        CHECK(fban >= bnns);
        CHECK(ban >= apples);
        CHECK(hi >= lo);
    }
    SECTION("gte - false") {
        CHECK_FALSE(apples >= ban);
        CHECK_FALSE(lo >= hi);
    }

    // TESTING <
    SECTION("lt - true") {
        CHECK(apples < ban);
    }
    SECTION("lt - false") {
        CHECK_FALSE(ban < apples);
    }
    SECTION("lt - filtered true") {
        CHECK(lo < hi);
    }
    SECTION("lt - filtered false") {
        CHECK_FALSE(hi < lo);
    }

    // TESTING <=
    SECTION("lte - true") {
        CHECK(fban <= bnns);
        CHECK(apples <= ban);
        CHECK(lo <= hi);
    }
    SECTION("lte - false") {
        CHECK_FALSE(ban <= apples);
        CHECK_FALSE(hi <= lo);
    }

    auto oss = std::ostringstream{};
    SECTION("output - full string") {
        auto view = fsv::filtered_string_view{"jumpingjacks"};
        oss << view;
        CHECK(oss.str() == "jumpingjacks");
    }
    SECTION("output - filtered") {
        auto view = fsv::filtered_string_view{"c++ > rust > java", [](const char& c) { return c == 'c' || c == '+'; }};
        oss << view;
        CHECK(oss.str() == "c++");
    }
}

TEST_CASE("MEMBER OPERATORS") {
    SECTION("copy - basic test") {
        auto pred = [](char c) { return c == '4' || c == '2'; };
        auto fsv1 = fsv::filtered_string_view{"42 meaning", pred};
        auto fsv2 = fsv::filtered_string_view{};
        fsv2 = fsv1;
        CHECK(fsv1 == fsv2);
        fsv1 = fsv1;
        CHECK(fsv1 == fsv2);
    }

    SECTION("move - basic test") {
        auto pred = [](char c) { return c == '1' || c == '2'; };
        auto fsv1 = fsv::filtered_string_view{"'12 zodiac", pred};
        auto fsv2 = fsv::filtered_string_view{};
        fsv2 = std::move(fsv1);
        CHECK(fsv2 == fsv::filtered_string_view{"'12 zodiac", pred});
        CHECK(fsv1.data() == nullptr);
        fsv2 = std::move(fsv2);
        CHECK(fsv2 == fsv::filtered_string_view{"'12 zodiac", pred});
    }

    SECTION("subscript - basic test") {
        auto pred = [](char c) { return c == '9' || c == '0' || c == ' '; };
        auto fsv1 = fsv::filtered_string_view{"only 90s kids understand", pred};

        auto result = std::string{};
        for (auto i = std::size_t{0}; i < fsv1.size(); ++i) {
            result += fsv1[i];
        }

        CHECK(result == " 90  ");
        CHECK(fsv1[2] == '0');
    }

    SECTION("explicit string conversion") {
        auto fsv = fsv::filtered_string_view("vizsla");
        auto s = static_cast<std::string>(fsv);

        CHECK(s == "vizsla");
        CHECK(s.data() != fsv.data());
    }
    SECTION("explicit string conversion - filtered") {
        auto pred = [](char c) { return std::isalpha(c); };
        auto fsv = fsv::filtered_string_view{"12Hello 3World!", pred};
        auto s = static_cast<std::string>(fsv);
        CHECK(s == "HelloWorld");
    }
}

TEST_CASE("ITERATOR") {
    SECTION("basic iteration - no filter") {
        auto s = fsv::filtered_string_view{"noor"};
        auto it = s.begin();

        REQUIRE(it != s.end());
        CHECK(*it == 'n');
        ++it;

        REQUIRE(it != s.end());
        CHECK(*it == 'o');
        ++it;

        REQUIRE(it != s.end());
        CHECK(*it == 'o');
        ++it;

        REQUIRE(it != s.end());
        CHECK(*it == 'r');
        ++it;

        REQUIRE(it == s.end());
    }

    SECTION("basic iteration - filter") {
        auto s = fsv::filtered_string_view{"youtube", [](const char& c) {
                                               return !(c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u');
                                           }};

        auto expected = std::vector<char>{'y', 't', 'b'};
        auto result = std::vector<char>{};

        for (auto ch : s) {
            result.push_back(ch);
        }

        CHECK(result == expected);
    }

    SECTION("iterator - std::prev") {
        const auto str = std::string("candle");
        const auto s = fsv::filtered_string_view{str};

        auto it = s.cend();

        CHECK(*std::prev(it) == 'e');
        CHECK(*std::prev(it, 2) == 'l');
    }

    SECTION("iterator reverse - rbegin() and rend()") {
        auto s = fsv::filtered_string_view{"odyssey", [](const char& c) { return !(c == 's' || c == 'y'); }};

        auto result = std::vector<char>(s.rbegin(), s.rend());
        auto expected = std::vector<char>{'e', 'd', 'o'};
        CHECK(result == expected);
    }

    SECTION("iterator reverse - no filter") {
        auto s = fsv::filtered_string_view{"superman"};
        auto result = std::vector<char>(s.rbegin(), s.rend());
        auto expected = std::vector<char>{'n', 'a', 'm', 'r', 'e', 'p', 'u', 's'};
        CHECK(result == expected);
    }

    SECTION("iterator - empty string") {
        auto s = fsv::filtered_string_view{""};
        CHECK(s.begin() == s.end());
        CHECK(s.rbegin() == s.rend());
    }

    SECTION("iterator - const and nonconst match begin/end") {
        auto s = fsv::filtered_string_view{"hello"};
        auto const& cs = s;

        CHECK(s.begin() == cs.cbegin());
        CHECK(s.end() == cs.cend());
        CHECK(s.rbegin() == cs.crbegin());
        CHECK(s.rend() == cs.crend());
    }
}

TEST_CASE("NON-MEMBER FUNCTIONS") {
    SECTION("compose") {
        auto base = fsv::filtered_string_view{"c / c++"};

        auto vf = std::vector<fsv::filter>{[](const char& c) { return c == 'c' || c == '+' || c == '/'; },
                                           [](const char& c) { return c > ' '; },
                                           [](const char&) { return true; }};

        auto oss = std::ostringstream{};
        oss << fsv::compose(base, vf);
        CHECK(oss.str() == "c/c++");
    }

    SECTION("substr - basic") {
        auto s = fsv::filtered_string_view{"new york city"};
        auto oss = std::ostringstream{};
        oss << fsv::substr(s, 4, 4);
        CHECK(oss.str() == "york");
    }
    SECTION("substr - position to end") {
        auto s = fsv::filtered_string_view{"baldurs gate"};
        auto result = fsv::substr(s, 8);
        REQUIRE(result.data() == s.data());
        auto oss = std::ostringstream{};
        oss << result;
        CHECK(oss.str() == "gate");
    }
    SECTION("substr - out of range") {
        auto s = fsv::filtered_string_view{"No no its no use jo, jo we gotta have it out",
                                           [](const char& c) { return c >= 'A' && c <= 'Z'; }};
        auto msg = "filtered_string_view::substr(6): position out of range for filtered string of size 1";
        REQUIRE_THROWS_AS(fsv::substr(s, 6), std::out_of_range);
        CHECK_THROWS_WITH(fsv::substr(s, 6), msg);
    }
    SECTION("substr - empty") {
        auto s = fsv::filtered_string_view{"notebook"};
        CHECK(fsv::substr(s, 8).empty());
        CHECK(fsv::substr(s, 3, 0).empty());
    }
    SECTION("substr - partial return") {
        auto s = fsv::filtered_string_view{"abcdefghijk"};
        auto result = fsv::substr(s, 2, 3);
        auto oss = std::ostringstream{};
        oss << result;
        assert(oss.str() == "cde");
    }

    SECTION("split - basic working test") {
        auto wentworth = fsv::filtered_string_view{"Malcom? Bligh? Turnbull", [](const char& c) { return c != '?'; }};
        auto token = fsv::filtered_string_view{" 2015", [](const char& c) { return c == ' '; }};
        auto representative = fsv::split(wentworth, token);
        CHECK(representative.at(1) == "Bligh");
    }
    SECTION("split - no tok (does not appear)") {
        auto fsv = fsv::filtered_string_view{"fishing"};
        auto tok = fsv::filtered_string_view{"robert"};
        auto result = fsv::split(fsv, tok);
        REQUIRE(result.size() == 1);
        CHECK(result.at(0) == "fishing");
    }
    SECTION("split - no tok (empty)") {
        auto fsv = fsv::filtered_string_view{"fishing"};
        auto empty_tok = fsv::filtered_string_view{""};
        auto result = fsv::split(fsv, empty_tok);
        REQUIRE(result.size() == 1);
        CHECK(result.at(0) == "fishing");
    }
    SECTION("split - tok at edges") {
        auto s = fsv::filtered_string_view{"xax"};
        auto tok = fsv::filtered_string_view{"x"};
        auto result = fsv::split(s, tok);
        auto expected = std::vector<fsv::filtered_string_view>{"", "a", ""};

        CHECK(result == expected);
    }
    SECTION("split - consecutive tok delimiters") {
        auto tok = fsv::filtered_string_view{"x", [](char c) { return c == 'x'; }};
        auto s = fsv::filtered_string_view{"xx", [](char c) { return c != ' '; }};
        auto result = fsv::split(s, tok);
        auto expected = std::vector<fsv::filtered_string_view>{"", "", ""};
        REQUIRE(result.size() == 3);
        CHECK(result == expected);
    }
}
