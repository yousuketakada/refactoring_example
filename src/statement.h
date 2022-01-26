#pragma once

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

struct Invoice
{
    struct Performance
    {
        std::string play_id;
        int audience;
    };

    std::string customer;
    std::vector<Performance> performances;
};

std::string statement(const Invoice& invoice, const std::map<std::string, Play>& plays);
