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

}

std::string statement(const Invoice& invoice, const std::map<std::string, Play>& plays)
{
    auto amount_for = [](const auto& perf, const auto& play)
    {
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
        return this_amount;
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
