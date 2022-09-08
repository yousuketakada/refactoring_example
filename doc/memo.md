# Refactoring Memo

The first chapter of Fowler (2018) gives a concrete example of
how we refactor code with some common anti-patterns or "bad smells."
Although the example is in JavaScript,
the concepts of refactoring are language agnostic so that
basically we can employ any other (dynamically- or statically-typed) language equally well;
see, e.g.,

* <https://github.com/ryo-utsunomiya/refactoring-2nd-ts>
  for TypeScript
* <https://github.com/emilybache/Theatrical-Players-Refactoring-Kata>
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
(all under the [`src`](../src) directory).

* [`statement_test.cpp`](../src/statement_test.cpp):
  The source file containing a simple test case named `BigCo`, in which
  we input some `plays` and `invoice` to a function `statement`;
  and compare the output with `expected_text` for equality.
* [`statement.h`](../src/statement.h):
  The header file where we define types `Play`, `Performance`, and `Invoice`;
  and declare `statement`.
* [`statement.cpp`](../src/statement.cpp):
  The source file where we define `statement`.

The data stored in JSON files like `plays.json` and `invoices.json`
for the original JavaScript example correspond to
the following constants found in `statement_test.cpp`
(Note the use of [designated initializers](https://en.cppreference.com/w/cpp/language/aggregate_initialization#Designated_initializers),
which gives the C++ code some resemblance with the original JSON code).

```cpp
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

```cpp
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

```cpp
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
we have already extracted a free function `usd` for formatting a monetary value into
a string like `$1,730.00`.
(The function `usd` makes use of a locale for monetary formatting,
which is, strictly speaking, not portable but seems to work just fine.)

At this moment, running tests of course gives a "green" output like the following
(see [README](../README.md#how-to-configure-build-and-test) for
how to configure the project with CMake).

```
$ cmake --build build && ctest --test-dir build --output-on-failure
[1/3] Building CXX object src\CMakeFiles\statement.dir\statement.cpp.obj
[2/3] Linking CXX static library src\statement.lib
[3/3] Linking CXX executable src\statement_test.exe
Internal ctest changing into directory: C:/Users/yousuke/work/refactoring_example/build
Test project C:/Users/yousuke/work/refactoring_example/build
    Start 1: StatementTest.BigCo
1/2 Test #1: StatementTest.BigCo ..............   Passed    0.04 sec
    Start 2: StatementTest.UnknownType
2/2 Test #2: StatementTest.UnknownType ........   Passed    0.02 sec

100% tests passed, 0 tests failed out of 2

Total Test time (real) =   0.08 sec
```

Note that the test case `UnknownType` verifies the behavior that
`statement` throws upon an unknown `Play::Type`.

If we introduce a bug, say, we forget to set (or `imbue`) the locale correctly in `usd`,
we indeed get an error or "red" output:

```
$ cmake --build build && ctest --test-dir build --output-on-failure
[1/3] Building CXX object src\CMakeFiles\statement.dir\statement.cpp.obj
[2/3] Linking CXX static library src\statement.lib
[3/3] Linking CXX executable src\statement_test.exe
Internal ctest changing into directory: C:/Users/yousuke/work/refactoring_example/build
Test project C:/Users/yousuke/work/refactoring_example/build
    Start 1: StatementTest.BigCo
1/2 Test #1: StatementTest.BigCo ..............***Failed    0.02 sec
Running main() from gmock_main.cc
Note: Google Test filter = StatementTest.BigCo
[==========] Running 1 test from 1 test suite.
[----------] Global test environment set-up.
[----------] 1 test from StatementTest
[ RUN      ] StatementTest.BigCo
C:\Users\yousuke\work\refactoring_example\src\statement_test.cpp(48): error: Expected equality of these values:
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
2/2 Test #2: StatementTest.UnknownType ........   Passed    0.02 sec

50% tests passed, 1 tests failed out of 2

Total Test time (real) =   0.06 sec

The following tests FAILED:
          1 - StatementTest.BigCo (Failed)
Errors while running CTest
```

As Fowler (2018) points out,
it is important to run tests often while we refactor:
A small refactoring step followed by compile-test-commit is
the basic rhythm of refactoring.

## Decomposing the `statement` function

The first refactoring we apply to `statement` is _Extract Function_.
Specifically, we extract the switch statement in the middle
that calculates the charge for a performance
to some new function, namely, `amount_for`.
Although the JavaScript example uses a nested function,
we have no such a thing in C++;
let us use a lambda here as the closest alternative:

```cpp
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
The first two variables `perf` and `play` are not modified so that we pass them as parameters;
the last one `this_amount` is the only modified variable (always initialized to zero)
so that we return it from the function.

Let us now consider where the variable `play` has come from:
`play` has been computed from `perf` so that there was actually no need to pass it
as a parameter at all.
_Extract Function_ can be less complicated (because less variables will go out of scope),
if we have removed such temporary variables in advance;
this is another useful refactoring called _Replace Temp with Query_.

We can apply _Replace Temp with Query_ to the variable `play`
in a series of refactoring moves.
We first extract the right hand side of the statement declaring `play` to a function,
say, `play_for`, after which we apply _Inline Variable_ to `play` to remove it.
Finally, we apply _Change Function Declaration_ to `amount_for` to remove the `play` parameter.
All of these yields:

```cpp
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
        oss << std::format(
            "  {}: {} ({} seats)\n"sv,
            play_for(perf).name, usd(this_amount), perf.audience);

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

```cpp
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
        oss << std::format(
            "  {}: {} ({} seats)\n"sv,
            play_for(perf).name, usd(amount_for(perf)), perf.audience);
    }

    oss << std::format("Amount owed is {}\n"sv, usd(total_amount()));
    oss << std::format("You earned {} credits\n"sv, total_volume_credits());
    return std::move(oss).str();
}
```

The top-level function `statement` now performs only formatting the statement
whereas the calculation logic has decomposed into nested functions (lambdas).

## Splitting the phases of calculation and formatting

Next, we apply _Split Phase_ to divide the `statement` function into two phases:
the first phase that calculates data required for the statement; and
the second phase that renders those calculated data into some particular format
(i.e., text for now but it is easy to support more formats such as HTML
if the two phases have been clearly separated).
To this end, we first extract the text rendering function `render_plain_text` from `statement`:

```cpp
struct StatementData {};

auto render_plain_text(
    [[maybe_unused]] const StatementData& data,
    const Invoice& invoice, const std::map<std::string, Play>& plays)
{
    ...
}
```

where the omitted function body is actually the same as that of the previous `statement` function
and let the new `statement` function simply call into `render_plain_text`:

```cpp
std::string statement(const Invoice& invoice, const std::map<std::string, Play>& plays)
{
    const StatementData statement_data;
    return render_plain_text(statement_data, invoice, plays);
}
```

Note that the extracted function `render_plain_text` takes
an empty parameter `data` of type `StatementData`,
to which we are going to put the intermediate data (the results of the first phase)
required for rendering the statement (the second phase)
so that `render_plain_text` depends only on `StatementData`.

It would be straightforward to simply put `customer` and `performances` of `Invoice`
to `StatementData` so as to remove the `invoice` parameter of `render_plain_text`.
In order to remove the `plays` parameter, however,
we need to somehow "enrich" each element of `performances`
so that, from the "enriched" performance, one can get the corresponding `play`
(this refactoring is unfortunately not given its name in the first chapter of Fowler (2018)
but is one of the most useful refactorings named _Combine Functions into Transform_
listed in Chapter 6: A First Set of Refactorings).
The function `render_plain_text` now takes only one parameter of type `StatementData`
that has been modified so as to contain "enriched" performances
each of which is of type `EnrichedPerformance`:

```cpp
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

auto render_plain_text(const StatementData& data)
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
        oss << std::format(
            "  {}: {} ({} seats)\n"sv,
            perf.play.name, usd(amount_for(perf)), perf.base.audience);
    }

    oss << std::format("Amount owed is {}\n"sv, usd(total_amount()));
    oss << std::format("You earned {} credits\n"sv, total_volume_credits());
    return std::move(oss).str();
}
```

The `statement` function populates `StatementData` and passes it to `render_plain_text`:

```cpp
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
    std::ranges::copy(
        invoice.performances | std::views::transform(enrich_performance),
        std::back_inserter(enriched_performances));
    assert(std::size(enriched_performances) == std::size(invoice.performances));

    const StatementData statement_data{
        .customer = invoice.customer,
        .performances = std::move(enriched_performances)
    };

    return render_plain_text(statement_data);
}
```

where we have moved the function `play_for` back to `statement` (_Move Function_)
in order for `enrich_performance` to "enrich" a performance with the corresponding `play`.

The "enrichment" could be done differently, e.g., by inheritance,
but here I have made `EnrichedPerformance` simply have a member named `base` that is
a reference to the original `Performance`.
This way, we can avoid deep copy at the cost that, for an "enriched" performance `perf`,
we have to say `perf.base.XYZ` to access the original member `XYZ`
(as we have done so in the above).

Note also that, in C++23, one can use
[`std::ranges::to`](https://en.cppreference.com/w/cpp/ranges/to)
to convert a range to a container
(in this case, `invoice.performances | std::views::transform(enrich_performance)` to
`std::vector<EnrichedPerformance>`),
but, in C++20, the most universal way to convert a range to an
[std::vector](https://en.cppreference.com/w/cpp/container/vector) would be to use
[std::back_insert_iterator](https://en.cppreference.com/w/cpp/iterator/back_insert_iterator)
as shown above
(see [here](https://timur.audio/how-to-make-a-container-from-a-c20-range)
for more discussions).

Let us further "enrich" `EnrichedPerformance`
with new fields `amount` and `volume_credits`;
we move `amount_for` and `volume_credits_for` again back to
the `statement` function to populate them.
To move back the remaining calculation functions,
i.e., `total_amount` and `total_volume_credits`,
we add the corresponding new fields to `StatementData`.
Making use of those new fields,
the function `render_plain_text` now does only formatting in its body as intended:

```cpp
struct EnrichedPerformance
{
    const Performance& base;
    const Play& play;
    int amount;
    int volume_credits;
};

struct StatementData
{
    const std::string& customer;
    std::vector<EnrichedPerformance> performances;
    int total_amount;
    int total_volume_credits;
};

auto render_plain_text(const StatementData& data)
{
    std::ostringstream oss;
    oss << std::format("Statement for {}\n"sv, data.customer);

    for (const auto& perf : data.performances) {
        oss << std::format(
            "  {}: {} ({} seats)\n"sv,
            perf.play.name, usd(perf.amount), perf.base.audience);
    }

    oss << std::format("Amount owed is {}\n"sv, usd(data.total_amount));
    oss << std::format("You earned {} credits\n"sv, data.total_volume_credits);
    return std::move(oss).str();
}
```

In `statement`, let us take this opportunity to remove raw accumulation loops
using an appropriate algorithm
(I wish we could use `std::ranges`-based reduction, which is yet to be standardized,
but I consider this refactoring a form of _Replace Loop with Pipeline_):

```cpp
std::string statement(const Invoice& invoice, const std::map<std::string, Play>& plays)
{
    auto play_for = [&](const auto& perf) -> decltype(auto)
    {
        return plays.at(perf.play_id);
    };

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

    auto enrich_performance = [&](const auto& base)
    {
        EnrichedPerformance enriched{
            .base = base,
            .play = play_for(base)
        };
        enriched.amount = amount_for(enriched);
        enriched.volume_credits = volume_credits_for(enriched);
        return enriched;
    };

    std::vector<EnrichedPerformance> enriched_performances;
    std::ranges::copy(
        invoice.performances | std::views::transform(enrich_performance),
        std::back_inserter(enriched_performances));
    assert(std::size(enriched_performances) == std::size(invoice.performances));

    const auto total_amount = std::accumulate(
        std::cbegin(enriched_performances), std::cend(enriched_performances),
        0, [](int sum, const auto& perf) { return sum + perf.amount; });
    const auto total_volume_credits = std::accumulate(
        std::cbegin(enriched_performances), std::cend(enriched_performances),
        0, [](int sum, const auto& perf) { return sum + perf.volume_credits; });

    const StatementData statement_data{
        .customer = invoice.customer,
        .performances = std::move(enriched_performances),
        .total_amount = total_amount,
        .total_volume_credits = total_volume_credits
    };

    return render_plain_text(statement_data);
}
```

We conclude our refactoring of splitting the phases by extracting the first phase
from the `statement` function to a new function `make_statement_data`:

```cpp
StatementData make_statement_data(const Invoice& invoice, const std::map<std::string, Play>& plays)
{
    auto play_for = [&](const auto& perf) -> decltype(auto)
    {
        return plays.at(perf.play_id);
    };

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

    auto enrich_performance = [&](const auto& base)
    {
        EnrichedPerformance enriched{
            .base = base,
            .play = play_for(base)
        };
        enriched.amount = amount_for(enriched);
        enriched.volume_credits = volume_credits_for(enriched);
        return enriched;
    };

    std::vector<EnrichedPerformance> enriched_performances;
    std::ranges::copy(
        invoice.performances | std::views::transform(enrich_performance),
        std::back_inserter(enriched_performances));
    assert(std::size(enriched_performances) == std::size(invoice.performances));

    const auto total_amount = std::accumulate(
        std::cbegin(enriched_performances), std::cend(enriched_performances),
        0, [](int sum, const auto& perf) { return sum + perf.amount; });
    const auto total_volume_credits = std::accumulate(
        std::cbegin(enriched_performances), std::cend(enriched_performances),
        0, [](int sum, const auto& perf) { return sum + perf.volume_credits; });

    return {
        .customer = invoice.customer,
        .performances = std::move(enriched_performances),
        .total_amount = total_amount,
        .total_volume_credits = total_volume_credits
    };
}
```

The `statement` function now reads:

```cpp
std::string statement(const Invoice& invoice, const std::map<std::string, Play>& plays)
{
    return render_plain_text(make_statement_data(invoice, plays));
}
```

Since the first phase has been clearly separated from the second,
let us also separate the source file to reflect the logical structure.
Specifically, we add new header and source files for `make_statement_data`
(and make `statement.cpp` where we define `statement` include that header):

* [`make_statement_data.h`](../src/make_statement_data.h):
  The header file where we define types `EnrichedPerformance` and `StatementData`; and
  declare `make_statement_data`.
* [`make_statement_data.cpp`](../src/make_statement_data.cpp):
  The source file where we define `make_statement_data`.

Now that the `statement` function is implemented simply by composing the two phases,
i.e., calculation and formatting,
one can easily implement an HTML version of `statement` by
composing the existing calculation phase and a new HTML formatting phase;
its implementation and test are omitted from this memo for brevity.

## Reorganizing the conditional logic on `Play::Type`

Lastly, let us consider refactorings required
when we add more `Play::Type`s and their calculation logic.
The functions (lambdas) `amount_for` and `volume_credits_for` defined in `make_statement_data`
contain some already complex conditional logic (i.e., `switch` and `if` statements)
on `Play::Type` for calculating data about performances;
such conditional logic can be represented naturally by using polymorphism,
making it easy to modify the logic or extend it with more categories.
Called _Replace Conditional with Polymorphism_,
this refactoring can be considered a form of the
[strategy pattern](https://en.wikipedia.org/wiki/Strategy_pattern)
because we shall dynamically select a suitable set of calculation algorithms
based upon `Play::Type` for each performance.

To apply _Replace Conditional with Polymorphism_,
we need some class inheritance hierarchy into which we reorganize the conditional logic.
As a first step, we create a stateless class named `PerformanceCalculator` and
move those functions that implement the calculation logic for performances,
i.e., `amount_for` and `volume_credits_for`, into it as member functions
(_Combine Functions into Class_):

```cpp
class PerformanceCalculator
{
public:
    int amount_for(const EnrichedPerformance& perf) const
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
    }

    int volume_credits_for(const EnrichedPerformance& perf) const
    {
        int volume_credits = 0;
        volume_credits += std::max(perf.base.audience - 30, 0);
        if (Play::Type::Comedy == perf.play.type) { volume_credits += perf.base.audience / 5; }
        return volume_credits;
    }
};
```

where we have made the return types and the parameter types of the member functions explicit
because we are going to declare them virtual
and make derived classes override them (`auto` types are ambiguous to inherit).
We then let `enrich_performance` in `make_statement_data` make use of `PerformanceCalculator`:

```cpp
StatementData make_statement_data(const Invoice& invoice, const std::map<std::string, Play>& plays)
{
    auto play_for = [&](const auto& perf) -> decltype(auto)
    {
        return plays.at(perf.play_id);
    };

    auto enrich_performance = [&](const auto& base)
    {
        EnrichedPerformance enriched{
            .base = base,
            .play = play_for(base)
        };
        const PerformanceCalculator calc;
        enriched.amount = calc.amount_for(enriched);
        enriched.volume_credits = calc.volume_credits_for(enriched);
        return enriched;
    };

    std::vector<EnrichedPerformance> enriched_performances;
    std::ranges::copy(
        invoice.performances | std::views::transform(enrich_performance),
        std::back_inserter(enriched_performances));
    assert(std::size(enriched_performances) == std::size(invoice.performances));

    const auto total_amount = std::accumulate(
        std::cbegin(enriched_performances), std::cend(enriched_performances),
        0, [](int sum, const auto& perf) { return sum + perf.amount; });
    const auto total_volume_credits = std::accumulate(
        std::cbegin(enriched_performances), std::cend(enriched_performances),
        0, [](int sum, const auto& perf) { return sum + perf.volume_credits; });

    return {
        .customer = invoice.customer,
        .performances = std::move(enriched_performances),
        .total_amount = total_amount,
        .total_volume_credits = total_volume_credits
    };
}
```

Next, we make `PerformanceCalculator` suitable for a base class
from which we derive a concrete calculator class
by declaring its members `amount_for` and `volume_credits_for` virtual and
its destructor protected
(so that it cannot be directly instantiated nor destructed except through its derived classes).
We then define derived calculators each for each `Play::Type`, namely,
`TragedyCalculator` for `Play::Type::Tragedy` and
`ComedyCalculator` for `Play::Type::Comedy`
(although they are empty for now, this is a step toward _Replace Type Code with Subclasses_):

```cpp
class PerformanceCalculator
{
public:
    virtual int amount_for(const EnrichedPerformance& perf) const
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
            assert(0);
        }
        return amount;
    }

    virtual int volume_credits_for(const EnrichedPerformance& perf) const
    {
        int volume_credits = 0;
        volume_credits += std::max(perf.base.audience - 30, 0);
        if (Play::Type::Comedy == perf.play.type) { volume_credits += perf.base.audience / 5; }
        return volume_credits;
    }

protected:
    ~PerformanceCalculator() = default;
};

class TragedyCalculator : public PerformanceCalculator {};
class ComedyCalculator : public PerformanceCalculator {};

const PerformanceCalculator& get_performance_calculator(Play::Type type)
{
    switch (type) {
    case Play::Type::Tragedy: { static const TragedyCalculator calc; return calc; }
    case Play::Type::Comedy: { static const ComedyCalculator calc; return calc; }
    }

    throw std::runtime_error{std::format(
        "{}: unknown Play::Type"sv,
        static_cast<std::underlying_type_t<Play::Type>>(type))};
}
```

We have also defined a factory function `get_performance_calculator`
that selects a suitable implementation of `PerformanceCalculator` based on `Play::Type`
or throws an exception if the type code is unknown
(the exception is adopted from `amount_for`).
We make use of this factory in `enrich_performance` in lieu of the constructor
(_Replace Constructor with Factory Function_):

```cpp
    auto enrich_performance = [&](const auto& base)
    {
        EnrichedPerformance enriched{
            .base = base,
            .play = play_for(base)
        };
        const auto& calc = get_performance_calculator(enriched.play.type);
        enriched.amount = calc.amount_for(enriched);
        enriched.volume_credits = calc.volume_credits_for(enriched);
        return enriched;
    };
```

Note that, unlike the original JavaScript example,
we have kept the derived calculators stateless
so that we can instantiate them statically in the factory
(otherwise we would have to dynamically allocate one, returning perhaps an `std::unique_ptr`);
they can be considered the simplest form of
[flyweight](https://en.wikipedia.org/wiki/Flyweight_pattern) objects.

TODO
