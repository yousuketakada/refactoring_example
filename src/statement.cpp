#include "pch.h"

#include "statement.h"

std::string statement(const std::string& name)
{
// #error XYZ
    return std::format("Hello, {}!\n"s, name);
}
