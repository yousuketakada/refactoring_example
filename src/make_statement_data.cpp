#include "pch.h"

#include "make_statement_data.h"

namespace {

struct PerformanceData
{
    const Performance& performance;
    const Play& play;
};

class PerformanceCalculator
{
public:
    int amount_for(const PerformanceData& data) const
    {
        int amount = 0;
        switch (data.play.type) {
        case Play::Type::Tragedy:
            amount = 40000;
            if (data.performance.audience > 30) {
                amount += 1000 * (data.performance.audience - 30);
            }
            break;
        case Play::Type::Comedy:
            amount = 30000;
            if (data.performance.audience > 20) {
                amount += 10000 + 500 * (data.performance.audience - 20);
            }
            amount += 300 * data.performance.audience;
            break;
        default:
            throw std::runtime_error{std::format(
                "unknown type: {}"s,
                static_cast<int>(data.play.type))};
        }
        return amount;
    }

    int volume_credits_for(const PerformanceData& data) const
    {
        int volume_credits = 0;
        volume_credits += std::max(data.performance.audience - 30, 0);
        if (Play::Type::Comedy == data.play.type) { volume_credits += data.performance.audience / 5; }
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

    auto enrich_performance = [&](const auto& perf)
    {
        const PerformanceData data{perf, play_for(perf)};
        const PerformanceCalculator calculator;

        return EnrichedPerformance{
            .base = perf,
            .play = data.play,
            .amount = calculator.amount_for(data),
            .volume_credits = calculator.volume_credits_for(data)
        };
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
