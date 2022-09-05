#include "pch.h"

#include "make_statement_data.h"

namespace {

class PerformanceCalculator
{
public:
    virtual int amount_for(const Performance& perf) const = 0;

    virtual int volume_credits_for(const Performance& perf) const
    {
        return std::max(perf.audience - 30, 0);
    }

protected:
    ~PerformanceCalculator() = default;
};

class TragedyCalculator : public PerformanceCalculator
{
    int amount_for(const Performance& perf) const override
    {
        int amount = 40000;
        if (perf.audience > 30) {
            amount += 1000 * (perf.audience - 30);
        }
        return amount;
    }
};

class ComedyCalculator : public PerformanceCalculator
{
    int amount_for(const Performance& perf) const override
    {
        int amount = 30000;
        if (perf.audience > 20) {
            amount += 10000 + 500 * (perf.audience - 20);
        }
        amount += 300 * perf.audience;
        return amount;
    }

    int volume_credits_for(const Performance& perf) const override
    {
        int volume_credits = PerformanceCalculator::volume_credits_for(perf);
        volume_credits += perf.audience / 5;
        return volume_credits;
    }
};

const PerformanceCalculator& get_performance_calculator(Play::Type type)
{
    switch (type) {
    case Play::Type::Tragedy: { static const TragedyCalculator calc; return calc; }
    case Play::Type::Comedy: { static const ComedyCalculator calc; return calc; }
    }

    throw std::runtime_error{std::format(
        "{}: unknown Play::Type"sv,
        static_cast<std::underlying_type_t<Play::Type>>(type))};
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
        const auto& calc = get_performance_calculator(play.type);

        return EnrichedPerformance{
            .base = perf,
            .play = play,
            .amount = calc.amount_for(perf),
            .volume_credits = calc.volume_credits_for(perf)
        };
    };

    std::vector<EnrichedPerformance> enriched_performances;
    std::ranges::copy(
        invoice.performances | std::views::transform(enrich_performance),
        std::back_inserter(enriched_performances));
    assert(std::size(enriched_performances) == std::size(invoice.performances));

    const auto total_amount = std::accumulate(
        std::cbegin(enriched_performances), std::cend(enriched_performances),
        0, [](int sum, const auto& perf) { return sum + perf.amount; });
    const auto total_volume_credits = std::accumulate(
        std::cbegin(enriched_performances), std::cend(enriched_performances),
        0, [](int sum, const auto& perf) { return sum + perf.volume_credits; });

    return {
        .customer = invoice.customer,
        .performances = std::move(enriched_performances),
        .total_amount = total_amount,
        .total_volume_credits = total_volume_credits
    };
}
