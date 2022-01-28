#include "pch.h"

#include "statement.h"

#include <gtest/gtest.h>

namespace {

TEST(StatementTest, BigCo)
{
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

    const auto actual_text = statement(invoice, plays);

    const auto expected_text = R"###(Statement for BigCo
  Hamlet: $650.00 (55 seats)
  As You Like It: $580.00 (35 seats)
  Othello: $500.00 (40 seats)
Amount owed is $1,730.00
You earned 47 credits
)###"s;

    EXPECT_EQ(actual_text, expected_text);
}

TEST(StatementTest, UnknownType)
{
    const std::map<std::string, Play> plays{
        {"xyz"s, {.name = "XYZ"s, .type = static_cast<Play::Type>(-1)}},
    };

    const Invoice invoice{
        .customer = "UT KK"s,
        .performances = {
            {
                .play_id = "xyz"s,
                .audience = 10
            },
        }
    };

    EXPECT_THROW([&]
    {
        try {
            statement(invoice, plays);
        }
        catch (const std::runtime_error& e) {
            EXPECT_EQ(e.what(), "-1: unknown Play::Type"s);
            throw;
        }
    } (), std::runtime_error);
}

}
