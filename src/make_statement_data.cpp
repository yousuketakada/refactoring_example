#include "pch.h"

#include "make_statement_data.h"

namespace {

struct PerformanceCalculator
{
    const Performance& performance;
    const Play& play;

    int calculate_amount() const
    {
        int amount = 0;
        switch (play.type) {
        case Play::Type::Tragedy:
            amount = 40000;
            if (performance.audience > 30) {
                amount += 1000 * (performance.audience - 30);
            }
            break;
        case Play::Type::Comedy:
            amount = 30000;
            if (performance.audience > 20) {
                amount += 10000 + 500 * (performance.audience - 20);
            }
            amount += 300 * performance.audience;
            break;
        default:
            throw std::runtime_error{std::format(
                "unknown type: {}"s,
                static_cast<int>(play.type))};
        }
        return amount;
    }

    int calculate_volume_credits() const
    {
        int volume_credits = 0;
        volume_credits += std::max(performance.audience - 30, 0);
        if (Play::Type::Comedy == play.type) { volume_credits += performance.audience / 5; }
        return volume_credits;
    }
};

}

StatementData make_statement_data(const Invoice& invoice, const std::map<std::string, Play>& plays)
{
    auto play_for = [&](const auto& perf) -> decltype(auto)
    {
        return plays.at(perf.play_id);
    };

    auto enrich_performance = [&](const auto& base)
    {
        const PerformanceCalculator calculator{base, play_for(base)};

        EnrichedPerformance enriched{
            .base = base,
            .play = calculator.play
        };

        enriched.amount = calculator.calculate_amount();
        enriched.volume_credits = calculator.calculate_volume_credits();

        return enriched;
    };

    std::vector<EnrichedPerformance> enriched_performances;
    std::ranges::copy(
        invoice.performances | std::views::transform(enrich_performance),
        std::back_inserter(enriched_performances));
    assert(std::size(enriched_performances) == std::size(invoice.performances));

    auto total_amount = std::accumulate(
        std::cbegin(enriched_performances), std::cend(enriched_performances),
        0, [](int sum, const auto& perf) { return sum + perf.amount; });
    auto total_volume_credits = std::accumulate(
        std::cbegin(enriched_performances), std::cend(enriched_performances),
        0, [](int sum, const auto& perf) { return sum + perf.volume_credits; });

    return StatementData{
        .customer = invoice.customer,
        .performances = std::move(enriched_performances),
        .total_amount = total_amount,
        .total_volume_credits = total_volume_credits
    };
}
