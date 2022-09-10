#include "pch.h"

#include "make_statement_data.h"

namespace {

class PerformanceCalculator
{
public:
    virtual int amount_for(const EnrichedPerformance& perf) const
    {
        int amount = 0;
        switch (perf.play.type) {
        case Play::Type::Tragedy:
            assert(0);
            break;
        case Play::Type::Comedy:
            amount = 30000;
            if (perf.base.audience > 20) {
                amount += 10000 + 500 * (perf.base.audience - 20);
            }
            amount += 300 * perf.base.audience;
            break;
        default:
            assert(0);
        }
        return amount;
    }

    virtual int volume_credits_for(const EnrichedPerformance& perf) const
    {
        int volume_credits = 0;
        volume_credits += std::max(perf.base.audience - 30, 0);
        if (Play::Type::Comedy == perf.play.type) { volume_credits += perf.base.audience / 5; }
        return volume_credits;
    }

protected:
    ~PerformanceCalculator() = default;
};

class TragedyCalculator : public PerformanceCalculator
{
public:
    int amount_for(const EnrichedPerformance& perf) const override
    {
        int amount = 40000;
        if (perf.base.audience > 30) {
            amount += 1000 * (perf.base.audience - 30);
        }
        return amount;
    }
};

class ComedyCalculator : public PerformanceCalculator {};

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

    auto enrich_performance = [&](const auto& base)
    {
        EnrichedPerformance enriched{
            .base = base,
            .play = play_for(base)
        };
        const auto& calc = get_performance_calculator(enriched.play.type);
        enriched.amount = calc.amount_for(enriched);
        enriched.volume_credits = calc.volume_credits_for(enriched);
        return enriched;
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
