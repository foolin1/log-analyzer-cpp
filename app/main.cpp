#include "log_parser.hpp"

#include <iostream>
#include <string_view>

int main()
{
    constexpr std::string_view sample_line =
        "2026-07-09T10:16:04 ERROR payments "
        "Payment failed after retry 840";

    const auto result =
        log_analyzer::LogParser::parse(sample_line);

    if (!result.success()) {
        std::cerr
            << "Parser error: "
            << result.error
            << '\n';

        return 1;
    }

    const auto& entry = *result.entry;

    std::cout << "Log Analyzer CLI 0.2.0\n";
    std::cout
        << "Level: "
        << log_analyzer::to_string(entry.level)
        << '\n';

    std::cout
        << "Service: "
        << entry.service
        << '\n';

    std::cout
        << "Message: "
        << entry.message
        << '\n';

    std::cout
        << "Duration: "
        << entry.duration_ms
        << " ms\n";

    return 0;
}