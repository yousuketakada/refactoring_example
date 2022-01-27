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

struct EnrichedPerformance
{
    const Invoice::Performance& base;
    const Play& play;
};

struct StatementData
{
    const std::string& customer;
    std::vector<EnrichedPerformance> performances;
};

std::string render_plain_text(const StatementData& data)
{
    auto amount_for = [&](const auto& perf)
    {
        int amount = 0;
        switch (perf.play.type) {
        case Play::Type::Tragedy:
            amount = 40000;
            if (perf.base.audience > 30) {
                amount += 1000 * (perf.base.audience - 30);
            }
            break;
        case Play::Type::Comedy:
            amount = 30000;
            if (perf.base.audience > 20) {
                amount += 10000 + 500 * (perf.base.audience - 20);
            }
            amount += 300 * perf.base.audience;
            break;
        default:
            throw std::runtime_error{std::format("unknown type: {}"s, static_cast<int>(perf.play.type))};
        }
        return amount;
    };

    auto volume_credits_for = [&](const auto& perf)
    {
        int volume_credits = 0;
        volume_credits += std::max(perf.base.audience - 30, 0);
        if (Play::Type::Comedy == perf.play.type) { volume_credits += perf.base.audience / 5; }
        return volume_credits;
    };

    auto total_amount = [&]()
    {
        int total = 0;
        for (const auto& perf : data.performances) {
            total += amount_for(perf);
        }
        return total;
    };

    auto total_volume_credits = [&]()
    {
        int total = 0;
        for (const auto& perf : data.performances) {
            total += volume_credits_for(perf);
        }
        return total;
    };

    std::ostringstream oss;
    oss << std::format("Statement for {}\n"s, data.customer);

    for (const auto& perf : data.performances) {
        oss << std::format("  {}: {} ({} seats)\n"s, perf.play.name, usd(amount_for(perf)), perf.base.audience);
    }

    oss << std::format("Amount owed is {}\n"s, usd(total_amount()));
    oss << std::format("You earned {} credits\n"s, total_volume_credits());
    return std::move(oss).str();
}

}

std::string statement(const Invoice& invoice, const std::map<std::string, Play>& plays)
{
    auto play_for = [&](const auto& perf) -> decltype(auto)
    {
        return plays.at(perf.play_id);
    };

    auto enrich_performance = [&](const auto& base)
    {
        return EnrichedPerformance{
            .base = base,
            .play = play_for(base)
        };
    };

    std::vector<EnrichedPerformance> enriched_performances;
    std::ranges::copy(invoice.performances | std::views::transform(enrich_performance), std::back_inserter(enriched_performances));

    const StatementData statement_data{
        .customer = invoice.customer,
        .performances = std::move(enriched_performances)
    };

    return render_plain_text(statement_data);
}
