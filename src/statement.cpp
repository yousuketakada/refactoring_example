#include "pch.h"

#include "statement.h"

namespace {

auto usd(int amount)
{
    std::ostringstream oss;
    oss.imbue(std::locale{"en_US"s});
    oss << std::showbase << std::put_money(amount);
    return std::move(oss).str();
}

struct StatementData
{
    const std::string& customer;
};

std::string render_plain_text(const StatementData& data, const Invoice& invoice, const std::map<std::string, Play>& plays)
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
            throw std::runtime_error{std::format("unknown type: {}"s, static_cast<int>(play_for(perf).type))};
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
    oss << std::format("Statement for {}\n"s, data.customer);

    for (const auto& perf : invoice.performances) {
        oss << std::format("  {}: {} ({} seats)\n"s, play_for(perf).name, usd(amount_for(perf)), perf.audience);
    }

    oss << std::format("Amount owed is {}\n"s, usd(total_amount()));
    oss << std::format("You earned {} credits\n"s, total_volume_credits());
    return std::move(oss).str();
}

}

std::string statement(const Invoice& invoice, const std::map<std::string, Play>& plays)
{
    const StatementData statement_data{
        invoice.customer,
    };

    return render_plain_text(statement_data, invoice, plays);
}
