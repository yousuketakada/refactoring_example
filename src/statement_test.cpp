#include "pch.h"

#include "statement.h"

namespace {

void assert_equal(const std::string& expected, const std::string& actual)
{
    if (actual != expected) {
        throw std::runtime_error{std::format(
            "strings do not match:\n"
            "vvvv expected vvvv\n"
            "{}\n"
            "^^^^ expected ^^^^\n"
            "vvvv actual vvvv\n"
            "{}\n"
            "^^^^ actual ^^^^\n"s,
            expected,
            actual)};
    }

    assert(actual == expected);
}

void statement_test()
{
    // const auto actual = statement("World"s);
    const auto actual = statement("Refactoring"s);

    const auto expected = "Hello, Refactoring!\n"s;

    assert_equal(expected, actual);
}

}

int main()
{
    try {
        statement_test();
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << std::format("error: {}\n"s, e.what());
    }

    return 1;
}
