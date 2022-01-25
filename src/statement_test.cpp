#include "pch.h"

#include "statement.h"

#include <gtest/gtest.h>

namespace {

TEST(StatementTest, PlainText)
{
    EXPECT_EQ(
        statement("Refactoring"s),
        "Hello, Refactoring!\n"s);
}

}
