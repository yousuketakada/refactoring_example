# Some Refactoring Memo

## Introduction

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

## The starting point

The source files relevant to our refactoring are the following (all under the `src` directory).

* `statement_test.cpp`:
  The source file containing a simple test case named `BigCo`, in which
  we input some `plays` and `invoice` to a function `statement()`;
  and compare the output with `expected_text` for equality.
* `statement.h`:
  The header file where we define types `Play`, `Performance`, and `Invoice`;
  and declare `statement()`.
* `statement.cpp`:
  The source file where we define `statement()`.

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
types like `Play` and `Invoice` must be defined and `statement()` be declared;
one can find those definitions and declaration in `statement.h`:

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

The function `statement()` defined in `statement.cpp` is also similar to the original JavaScript:

```C++
namespace {

auto usd(int amount)
{
    std::ostringstream oss;
    oss.imbue(std::locale{"en_US"s});
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
                static_cast<int>(play.type))};
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
we have already extracted a free function `usd()` for formatting money like `$1,730.00`.
(The function `usd()` makes use of a locale for formatting money,
which is, strictly speaking, not portable but seems to work just fine.)

TODO

## Extract functions

TODO

## Split phase

TODO

## Replace conditionals with polymorphism

TODO

## Discussions

TODO
