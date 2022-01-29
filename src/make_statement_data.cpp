#include "pch.h"

#include "make_statement_data.h"

namespace {

class PerformanceCalculator
{
public:
    virtual int amount_for(const Performance& performance) const = 0;

    virtual int volume_credits_for(const Performance& performance) const
    {
        return std::max(performance.audience - 30, 0);
    }

protected:
    ~PerformanceCalculator() = default;
};

class TragedyPerformanceCalculator : public PerformanceCalculator
{
    int amount_for(const Performance& performance) const override
    {
        int amount = 40000;
        if (performance.audience > 30) {
            amount += 1000 * (performance.audience - 30);
        }
        return amount;
    }
};

class ComedyPerformanceCalculator : public PerformanceCalculator
{
    int amount_for(const Performance& performance) const override
    {
        int amount = 30000;
        if (performance.audience > 20) {
            amount += 10000 + 500 * (performance.audience - 20);
        }
        amount += 300 * performance.audience;
        return amount;
    }

    int volume_credits_for(const Performance& performance) const override
    {
        int volume_credits = PerformanceCalculator::volume_credits_for(performance);
        volume_credits += performance.audience / 5;
        return volume_credits;
    }
};

const PerformanceCalculator& get_performance_calculator(Play::Type type)
{
    switch (type) {
#define CASE_FOR_PLAY_TYPE(X) \
case Play::Type::X: { static const X ## PerformanceCalculator calculator; return calculator; }

    CASE_FOR_PLAY_TYPE(Tragedy)
    CASE_FOR_PLAY_TYPE(Comedy)

#undef CASE_FOR_PLAY_TYPE

    default:
        throw std::runtime_error{std::format("{}: unknown Play::Type"s, static_cast<int>(type))};
    }
}

}

StatementData make_statement_data(const Invoice& invoice, const std::map<std::string, Play>& plays)
{
    auto play_for = [&](const auto& perf) -> decltype(auto)
    {
        return plays.at(perf.play_id);
    };

    auto enrich_performance = [&](const auto& perf)
    {
        const auto& play = play_for(perf);
        const auto& calculator = get_performance_calculator(play.type);

        return EnrichedPerformance{
            .base = perf,
            .play = play,
            .amount = calculator.amount_for(perf),
            .volume_credits = calculator.volume_credits_for(perf)
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