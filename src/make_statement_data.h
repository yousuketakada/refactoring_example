#pragma once

#include "statement.h"

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

StatementData make_statement_data(const Invoice& invoice, const std::map<std::string, Play>& plays);
