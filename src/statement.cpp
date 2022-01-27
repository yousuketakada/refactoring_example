#include "pch.h"

#include "statement.h"

#include "make_statement_data.h"

namespace {

auto usd(int amount)
{
    std::ostringstream oss;
    oss.imbue(std::locale{"en_US"s});
    oss << std::showbase << std::put_money(amount);
    return std::move(oss).str();
}

std::string render_plain_text(const StatementData& data)
{
    std::ostringstream oss;
    oss << std::format("Statement for {}\n"s, data.customer);

    for (const auto& perf : data.performances) {
        oss << std::format(
            "  {}: {} ({} seats)\n"s,
            perf.play.name, usd(perf.amount), perf.base.audience);
    }

    oss << std::format("Amount owed is {}\n"s, usd(data.total_amount));
    oss << std::format("You earned {} credits\n"s, data.total_volume_credits);
    return std::move(oss).str();
}

}

std::string statement(const Invoice& invoice, const std::map<std::string, Play>& plays)
{
    return render_plain_text(make_statement_data(invoice, plays));
}
