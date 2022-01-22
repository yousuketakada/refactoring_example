#include "pch.h"

#include "statement.h"

std::string statement(const std::string& name)
{
    return std::format("Hello, {}!\n"s, name);
}
