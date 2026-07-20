#include "log_filter.hpp"
#include "log_reader.hpp"

#include <filesystem>
#include <iostream>

int main()
{
    const std::filesystem::path input_path{
        "samples/application.log"
    };

    const auto read_result =
        log_analyzer::LogReader::read(input_path);

    if (!read_result.success()) {
        std::cerr
            << "Reader error: "
            << read_result.error
            << '\n';

        return 2;
    }

    const auto selected_date =
        log_analyzer::parse_filter_date(
            "2026-07-09");

    if (!selected_date.has_value()) {
        std::cerr
            << "Internal error: invalid filter date\n";

        return 1;
    }

    log_analyzer::LogFilterOptions options;
    options.level =
        log_analyzer::LogLevel::Error;
    options.service = "payments";
    options.from_date = selected_date;
    options.to_date = selected_date;

    const auto filtered_entries =
        log_analyzer::LogFilter::apply(
            read_result.entries,
            options);

    std::cout << "Log Analyzer CLI 0.4.0\n";

    std::cout
        << "Log file: "
        << input_path.string()
        << '\n';

    std::cout
        << "Parsed entries: "
        << read_result.entries.size()
        << '\n';

    std::cout
        << "Invalid lines: "
        << read_result.invalid_lines
        << '\n';

    std::cout
        << "Filtered entries: "
        << filtered_entries.size()
        << '\n';

    std::cout << "Selected filters:\n";
    std::cout << " Level: ERROR\n";
    std::cout << " Service: payments\n";
    std::cout << " Date: 2026-07-09\n";

    for (const auto& entry : filtered_entries) {
        std::cout
            << " - "
            << entry.message
            << " ("
            << entry.duration_ms
            << " ms)\n";
    }

    return 0;
}