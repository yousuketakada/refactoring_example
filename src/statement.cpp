#include "pch.h"

#include "statement.h"

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
        // add volume credits
        volume_credits += std::max(perf.audience - 30, 0);
        // add extra credit for every ten comedy attendees
        if (Play::Type::Comedy == play_for(perf).type) { volume_credits += perf.audience / 5; }

        // print line for this order
        oss << std::format("  {}: {} ({} seats)\n"sv, play_for(perf).name, usd(amount_for(perf)), perf.audience);
        total_amount += amount_for(perf);
    }

    oss << std::format("Amount owed is {}\n"sv, usd(total_amount));
    oss << std::format("You earned {} credits\n"sv, volume_credits);
    return std::move(oss).str();
}
