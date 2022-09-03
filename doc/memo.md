# Refactoring Memo

The first chapter of Fowler (2018) gives a concrete example of
how we refactor code with some common anti-patterns or "bad smells."
Although the example is in JavaScript,
the concepts of refactoring are language agnostic so that
basically we can employ any other (dynamically- or statically-typed) language equally well;
see, e.g.,

* https://github.com/ryo-utsunomiya/refactoring-2nd-ts
  for TypeScript
* https://github.com/emilybache/Theatrical-Players-Refactoring-Kata
  for a handful of other languages including C++, C#, Go, Java, Python, etc.

This repo is yet another example in C++20,
where the `master` branch corresponds to "the starting point" and
`refactored` to the fully refactored state.
Comparing `refactored` with `master`,
one can see a typical process of refactoring.
As demonstrated in Fowler (2018),
in order to minimize the risks of refactoring
(introducing subtle bugs, breaking the code for a long time, etc.),
I have tried to refactor with the "one small step at a time" principle in mind
and made as frequent commits as possible (without squashing).
In what follows, I shall relate a few significant steps of refactoring,
mainly focusing on the difference between JavaScript and C++.

## The starting point

The source files relevant to our refactoring are the following
(all under the [`src`](/src) directory).

* [`statement_test.cpp`](/src/statement_test.cpp):
  The source file containing a simple test case named `BigCo`, in which
  we input some `plays` and `invoice` to a function `statement`;
  and compare the output with `expected_text` for equality.
* [`statement.h`](/src/statement.h):
  The header file where we define types `Play`, `Performance`, and `Invoice`;
  and declare `statement`.
* [`statement.cpp`](/src/statement.cpp):
  The source file where we define `statement`.

The data stored in JSON files like `plays.json` and `invoices.json`
for the original JavaScript example correspond to
the following constants found in `statement_test.cpp`
(Note the use of [designated initializers](https://en.cppreference.com/w/cpp/language/aggregate_initialization#Designated_initializers),
which gives the C++ code some resemblance with the original JSON code).

```C++
    const std::map<std::string, Play> plays{
        {"hamlet"s, {.name = "Hamlet"s, .type = Play::Type::Tragedy}},
        {"as-like"s, {.name = "As You Like It"s, .type = Play::Type::Comedy}},
        {"othello"s, {.name = "Othello"s, .type = Play::Type::Tragedy}},
    };

    const Invoice invoice{
        .customer = "BigCo"s,
        .performances = {
            {
                .play_id = "hamlet"s,
                .audience = 55
            },
            {
                .play_id = "as-like"s,
                .audience = 35
            },
            {
                .play_id = "othello"s,
                .audience = 40
            },
        }
    };
```

With those constants, `statement(invoice, plays)` gives the same expected text:

```
Statement for BigCo
  Hamlet: $650.00 (55 seats)
  As You Like It: $580.00 (35 seats)
  Othello: $500.00 (40 seats)
Amount owed is $1,730.00
You earned 47 credits
```

Since C++ is statically-typed,
types like `Play` and `Invoice` must be defined and `statement` be declared;
one can find those definitions and declaration in `statement.h`
(Note also that we define the play type `Play::Type` as an enum class rather than string):

```C++
struct Play
{
    enum class Type
    {
        Tragedy,
        Comedy,
    };

    std::string name;
    Type type;
};

struct Performance
{
    std::string play_id;
    int audience;
};

struct Invoice
{
    std::string customer;
    std::vector<Performance> performances;
};

std::string statement(const Invoice& invoice, const std::map<std::string, Play>& plays);
```

The function `statement` defined in `statement.cpp` is also similar to the original JavaScript:

```C++
namespace {

auto usd(int amount)
{
    std::ostringstream oss;
    oss.imbue(std::locale{"en_US.UTF-8"s});
    oss << std::showbase << std::put_money(amount);
    return std::move(oss).str();
}

}

std::string statement(const Invoice& invoice, const std::map<std::string, Play>& plays)
{
    int total_amount = 0;
    int volume_credits = 0;
    std::ostringstream oss;
    oss << std::format("Statement for {}\n"sv, invoice.customer);

    for (const auto& perf : invoice.performances) {
        const auto& play = plays.at(perf.play_id);
        int this_amount = 0;

        switch (play.type) {
        case Play::Type::Tragedy:
            this_amount = 40000;
            if (perf.audience > 30) {
                this_amount += 1000 * (perf.audience - 30);
            }
            break;
        case Play::Type::Comedy:
            this_amount = 30000;
            if (perf.audience > 20) {
                this_amount += 10000 + 500 * (perf.audience - 20);
            }
            this_amount += 300 * perf.audience;
            break;
        default:
            throw std::runtime_error{std::format(
                "{}: unknown Play::Type"sv,
                static_cast<std::underlying_type_t<Play::Type>>(play.type))};
        }

        // add volume credits
        volume_credits += std::max(perf.audience - 30, 0);
        // add extra credit for every ten comedy attendees
        if (Play::Type::Comedy == play.type) { volume_credits += perf.audience / 5; }

        // print line for this order
        oss << std::format("  {}: {} ({} seats)\n"sv, play.name, usd(this_amount), perf.audience);
        total_amount += this_amount;
    }

    oss << std::format("Amount owed is {}\n"sv, usd(total_amount));
    oss << std::format("You earned {} credits\n"sv, volume_credits);
    return std::move(oss).str();
}
```

Note however that, unlike the original JavaScript,
we have already extracted a free function `usd` for formatting money like `$1,730.00`.
(The function `usd` makes use of a locale for formatting money,
which is, strictly speaking, not portable but seems to work just fine.)

At this moment, running tests of course gives a "green" output like the following
(see [README](/README.md#how-to-configure-build-and-test) for
how to configure the project with CMake).

```
$ cmake --build build && ctest --test-dir build --output-on-failure
[1/3] Building CXX object src\CMakeFiles\statement.dir\statement.cpp.obj
[2/3] Linking CXX static library src\statement.lib
[3/3] Linking CXX executable src\statement_test.exe
Internal ctest changing into directory: C:/Users/yousuke/work/refactoring_example/build
Test project C:/Users/yousuke/work/refactoring_example/build
    Start 1: StatementTest.BigCo
1/2 Test #1: StatementTest.BigCo ..............   Passed    0.03 sec
    Start 2: StatementTest.UnknownType
2/2 Test #2: StatementTest.UnknownType ........   Passed    0.01 sec

100% tests passed, 0 tests failed out of 2

Total Test time (real) =   0.06 sec
```

If we introduce a bug, say, we forget to set the locale correctly in `usd`,
we indeed get an error or "red" output like the following:

```
$ cmake --build build && ctest --test-dir build --output-on-failure
[1/3] Building CXX object src\CMakeFiles\statement.dir\statement.cpp.obj
[2/3] Linking CXX static library src\statement.lib
[3/3] Linking CXX executable src\statement_test.exe
Internal ctest changing into directory: C:/Users/yousuke/work/refactoring_example/build
Test project C:/Users/yousuke/work/refactoring_example/build
    Start 1: StatementTest.BigCo
1/2 Test #1: StatementTest.BigCo ..............***Failed    0.01 sec
Running main() from C:\Users\yousuke\work\refactoring_example\build\_deps\googletest-src\googletest\src\gtest_main.cc
Note: Google Test filter = StatementTest.BigCo
[==========] Running 1 test from 1 test suite.
[----------] Global test environment set-up.
[----------] 1 test from StatementTest
[ RUN      ] StatementTest.BigCo
C:\Users\yousuke\work\refactoring_example\src\statement_test.cpp(45): error: Expected equality of these values:
  actual_text
    Which is: "Statement for BigCo\n  Hamlet: 65000 (55 seats)\n  As You Like It: 58000 (35 seats)\n  Othello: 50000 (40 seats)\nAmount owed is 173000\nYou earned 47 credits\n"
  expected_text
    Which is: "Statement for BigCo\n  Hamlet: $650.00 (55 seats)\n  As You Like It: $580.00 (35 seats)\n  Othello: $500.00 (40 seats)\nAmount owed is $1,730.00\nYou earned 47 credits\n"
With diff:
@@ -1,6 +1,6 @@
 Statement for BigCo
-  Hamlet: 65000 (55 seats)
-  As You Like It: 58000 (35 seats)
-  Othello: 50000 (40 seats)
-Amount owed is 173000
+  Hamlet: $650.00 (55 seats)
+  As You Like It: $580.00 (35 seats)
+  Othello: $500.00 (40 seats)
+Amount owed is $1,730.00
 You earned 47 credits\n

[  FAILED  ] StatementTest.BigCo (1 ms)
[----------] 1 test from StatementTest (1 ms total)

[----------] Global test environment tear-down
[==========] 1 test from 1 test suite ran. (1 ms total)
[  PASSED  ] 0 tests.
[  FAILED  ] 1 test, listed below:
[  FAILED  ] StatementTest.BigCo

 1 FAILED TEST

    Start 2: StatementTest.UnknownType
2/2 Test #2: StatementTest.UnknownType ........   Passed    0.01 sec

50% tests passed, 1 tests failed out of 2

Total Test time (real) =   0.04 sec

The following tests FAILED:
          1 - StatementTest.BigCo (Failed)
Errors while running CTest
```

As Fowler (2018) points out,
it is important to run tests often while we refactor.

## Decomposing the `statement` function

The first refactoring we apply to `statement` is _Extract Function_.
Specifically, we extract the switch statement in the middle
that calculates the charge for a performance
to some new function, namely, `amount_for`.
Although the JavaScript example uses a nested function,
we have no such a thing in C++;
let us use a lambda here as the closest alternative:

```C++
std::string statement(const Invoice& invoice, const std::map<std::string, Play>& plays)
{
    auto amount_for = [](const auto& perf, const auto& play)
    {
        int amount = 0;
        switch (play.type) {
        case Play::Type::Tragedy:
            amount = 40000;
            if (perf.audience > 30) {
                amount += 1000 * (perf.audience - 30);
            }
            break;
        case Play::Type::Comedy:
            amount = 30000;
            if (perf.audience > 20) {
                amount += 10000 + 500 * (perf.audience - 20);
            }
            amount += 300 * perf.audience;
            break;
        default:
            throw std::runtime_error{std::format(
                "{}: unknown Play::Type"sv,
                static_cast<std::underlying_type_t<Play::Type>>(play.type))};
        }
        return amount;
    };

    int total_amount = 0;
    int volume_credits = 0;
    std::ostringstream oss;
    oss << std::format("Statement for {}\n"sv, invoice.customer);

    for (const auto& perf : invoice.performances) {
        const auto& play = plays.at(perf.play_id);
        int this_amount = amount_for(perf, play);

        // add volume credits
        volume_credits += std::max(perf.audience - 30, 0);
        // add extra credit for every ten comedy attendees
        if (Play::Type::Comedy == play.type) { volume_credits += perf.audience / 5; }

        // print line for this order
        oss << std::format("  {}: {} ({} seats)\n"sv, play.name, usd(this_amount), perf.audience);
        total_amount += this_amount;
    }

    oss << std::format("Amount owed is {}\n"sv, usd(total_amount));
    oss << std::format("You earned {} credits\n"sv, volume_credits);
    return std::move(oss).str();
}
```

When we did the extraction to the function `amount_for`
(which is to be precise a lambda but we no longer distinguish between lambda and function),
we had to deal with three variables
`perf`, `play`, and `this_amount` that would go out of scope.
The first two variables `perf` and `play` are not modified so that we pass them as the parameters;
the last one `this_amount` is the only modified variable so that we return it from the function.

Let us now consider where the variable `play` has come from:
`play` is computed from `perf` so that there was actually no need to pass it
as a parameter at all.
_Extract Function_ can be less complicated (because less variables will go out of scope)
if we have removed such temporary variables in advance,
which is another useful refactoring called _Replace Temp with Query_.

We can apply _Replace Temp with Query_ to the variable `play`
in a series of refactoring moves.
We first extract the right hand side of the statement declaring `play` to a function,
say, `play_for`, after which we apply _Inline Variable_ to `play` to remove it.
Finally, we apply _Change Function Declaration_ to `amount_for` to remove the `play` parameter.
All of these yields:

```C++
std::string statement(const Invoice& invoice, const std::map<std::string, Play>& plays)
{
    auto play_for = [&](const auto& perf) -> decltype(auto)
    {
        return plays.at(perf.play_id);
    };

    auto amount_for = [&](const auto& perf)
    {
        int amount = 0;
        switch (play_for(perf).type) {
        case Play::Type::Tragedy:
            amount = 40000;
            if (perf.audience > 30) {
                amount += 1000 * (perf.audience - 30);
            }
            break;
        case Play::Type::Comedy:
            amount = 30000;
            if (perf.audience > 20) {
                amount += 10000 + 500 * (perf.audience - 20);
            }
            amount += 300 * perf.audience;
            break;
        default:
            throw std::runtime_error{std::format(
                "{}: unknown Play::Type"sv,
                static_cast<std::underlying_type_t<Play::Type>>(play_for(perf).type))};
        }
        return amount;
    };

    int total_amount = 0;
    int volume_credits = 0;
    std::ostringstream oss;
    oss << std::format("Statement for {}\n"sv, invoice.customer);

    for (const auto& perf : invoice.performances) {
        int this_amount = amount_for(perf);

        // add volume credits
        volume_credits += std::max(perf.audience - 30, 0);
        // add extra credit for every ten comedy attendees
        if (Play::Type::Comedy == play_for(perf).type) { volume_credits += perf.audience / 5; }

        // print line for this order
        oss << std::format("  {}: {} ({} seats)\n"sv, play_for(perf).name, usd(this_amount), perf.audience);
        total_amount += this_amount;
    }

    oss << std::format("Amount owed is {}\n"sv, usd(total_amount));
    oss << std::format("You earned {} credits\n"sv, volume_credits);
    return std::move(oss).str();
}
```

Since we have eliminated the variable `play`,
we can now easily extract the function `volume_credits_for`
that calculates the volume credits for a performance.
_Replace Temp with Query_ also applies to `this_amount`.
Similarly, we apply _Extract Function_ and _Replace Temp with Query_
to the function scope variables `total_amount` and `volume_credits`.
Finally we have:

```C++
std::string statement(const Invoice& invoice, const std::map<std::string, Play>& plays)
{
    auto play_for = [&](const auto& perf) -> decltype(auto)
    {
        return plays.at(perf.play_id);
    };

    auto amount_for = [&](const auto& perf)
    {
        int amount = 0;
        switch (play_for(perf).type) {
        case Play::Type::Tragedy:
            amount = 40000;
            if (perf.audience > 30) {
                amount += 1000 * (perf.audience - 30);
            }
            break;
        case Play::Type::Comedy:
            amount = 30000;
            if (perf.audience > 20) {
                amount += 10000 + 500 * (perf.audience - 20);
            }
            amount += 300 * perf.audience;
            break;
        default:
            throw std::runtime_error{std::format(
                "{}: unknown Play::Type"sv,
                static_cast<std::underlying_type_t<Play::Type>>(play_for(perf).type))};
        }
        return amount;
    };

    auto volume_credits_for = [&](const auto& perf)
    {
        int volume_credits = 0;
        volume_credits += std::max(perf.audience - 30, 0);
        if (Play::Type::Comedy == play_for(perf).type) { volume_credits += perf.audience / 5; }
        return volume_credits;
    };

    auto total_amount = [&]()
    {
        int total = 0;
        for (const auto& perf : invoice.performances) {
            total += amount_for(perf);
        }
        return total;
    };

    auto total_volume_credits = [&]()
    {
        int total = 0;
        for (const auto& perf : invoice.performances) {
            total += volume_credits_for(perf);
        }
        return total;
    };

    std::ostringstream oss;
    oss << std::format("Statement for {}\n"sv, invoice.customer);

    for (const auto& perf : invoice.performances) {
        oss << std::format("  {}: {} ({} seats)\n"sv, play_for(perf).name, usd(amount_for(perf)), perf.audience);
    }

    oss << std::format("Amount owed is {}\n"sv, usd(total_amount()));
    oss << std::format("You earned {} credits\n"sv, total_volume_credits());
    return std::move(oss).str();
}
```

The top-level function `statement` now performs only printing the statement
whereas the calculation logic has decomposed into nested functions (lambdas).

## Splitting phases of calculation and formatting

Next, we apply _Split Phase_ to divide `statement` into two phases:
the first phase that calculates data required for the statement; and
the second phase that renders it into some particular format
(i.e., text for now but it is easy to support more formats
if the two phases are clearly separated).
To this end, we extract the text rendering function `render_plain_text` from `statement`:

```C++
struct StatementData {};

std::string render_plain_text([[maybe_unused]] const StatementData& data, const Invoice& invoice, const std::map<std::string, Play>& plays)
{
    ...
}
```

where the omitted function body is actually the same as that of the previous `statement` function
and let the new `statement` function call into `render_plain_text`:

```C++
std::string statement(const Invoice& invoice, const std::map<std::string, Play>& plays)
{
    const StatementData statement_data;
    return render_plain_text(statement_data, invoice, plays);
}
```

Note that the extracted function `render_plain_text` takes
an empty parameter `data` of type `StatementData`,
to which we are going to put the intermediate data (the results of the first phase)
required to render the statement (the second phase)
so that `render_plain_text` depends only on `StatementData`.

It would be straightforward to simply put `customer` and `performances` of `Invoice`
to `StatementData` so as to remove the `invoice` parameter of `render_plain_text`.
In order to remove the `plays` parameter, however,
we need to somehow "enrich" each element of `performances`
so that, from the "enriched" performance, one can get the corresponding `play`
(this refactoring is unfortunately not given its name in the first chapter of Fowler (2018)
but is one the most useful refactorings named _Combine Functions into Transform_
listed in Chapter 6: A First Set of Refactorings).
The function `render_plain_text` now takes only one parameter of type `StatementData`
that has been modified so as to contain "enriched" performances of type `EnrichedPerformance`:

```C++
struct EnrichedPerformance
{
    const Performance& base;
    const Play& play;
};

struct StatementData
{
    const std::string& customer;
    std::vector<EnrichedPerformance> performances;
};

std::string render_plain_text(const StatementData& data)
{
    auto amount_for = [&](const auto& perf)
    {
        int amount = 0;
        switch (perf.play.type) {
        case Play::Type::Tragedy:
            amount = 40000;
            if (perf.base.audience > 30) {
                amount += 1000 * (perf.base.audience - 30);
            }
            break;
        case Play::Type::Comedy:
            amount = 30000;
            if (perf.base.audience > 20) {
                amount += 10000 + 500 * (perf.base.audience - 20);
            }
            amount += 300 * perf.base.audience;
            break;
        default:
            throw std::runtime_error{std::format(
                "{}: unknown Play::Type"sv,
                static_cast<std::underlying_type_t<Play::Type>>(perf.play.type))};
        }
        return amount;
    };

    auto volume_credits_for = [&](const auto& perf)
    {
        int volume_credits = 0;
        volume_credits += std::max(perf.base.audience - 30, 0);
        if (Play::Type::Comedy == perf.play.type) { volume_credits += perf.base.audience / 5; }
        return volume_credits;
    };

    auto total_amount = [&]()
    {
        int total = 0;
        for (const auto& perf : data.performances) {
            total += amount_for(perf);
        }
        return total;
    };

    auto total_volume_credits = [&]()
    {
        int total = 0;
        for (const auto& perf : data.performances) {
            total += volume_credits_for(perf);
        }
        return total;
    };

    std::ostringstream oss;
    oss << std::format("Statement for {}\n"sv, data.customer);

    for (const auto& perf : data.performances) {
        oss << std::format("  {}: {} ({} seats)\n"sv, perf.play.name, usd(amount_for(perf)), perf.base.audience);
    }

    oss << std::format("Amount owed is {}\n"sv, usd(total_amount()));
    oss << std::format("You earned {} credits\n"sv, total_volume_credits());
    return std::move(oss).str();
}
```

The `statement` function populates `StatementData` and passes it to `render_plain_text`:

```C++
std::string statement(const Invoice& invoice, const std::map<std::string, Play>& plays)
{
    auto play_for = [&](const auto& perf) -> decltype(auto)
    {
        return plays.at(perf.play_id);
    };

    auto enrich_performance = [&](const auto& base)
    {
        return EnrichedPerformance{
            .base = base,
            .play = play_for(base)
        };
    };

    std::vector<EnrichedPerformance> enriched_performances;
    std::ranges::copy(invoice.performances | std::views::transform(enrich_performance), std::back_inserter(enriched_performances));

    const StatementData statement_data{
        .customer = invoice.customer,
        .performances = std::move(enriched_performances)
    };

    return render_plain_text(statement_data);
}
```

where we have moved the function `play_for` back to `statement`
in order for `enrich_performance` to "enrich" a performance with the corresponding `play`.

The "enrichment" could be done differently, e.g., by inheritance,
but here I have made `EnrichedPerformance` simply have a member named `base` that is
a reference to the original `Performance`.
This way, we can avoid a deep copy at the cost that we have to say `perf.base.XYZ`
to access the original member `XYZ` (as we have done so in `render_plain_text`).

TODO

## Reorganizing conditional logic with polymorphism (strategy pattern)

TODO
