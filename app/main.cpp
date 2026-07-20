#include "log_filter.hpp"
#include "log_level.hpp"
#include "log_reader.hpp"
#include "log_statistics.hpp"

#include <cstddef>
#include <filesystem>
#include <iomanip>
#include <iostream>

namespace {

void print_level_count(
    const log_analyzer::LogStatisticsResult& statistics,
    const log_analyzer::LogLevel level)
{
    std::cout
        << ' '
        << log_analyzer::to_string(level)
        << ": "
        << statistics.level_counts.at(level)
        << '\n';
}

} // namespace

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

    const log_analyzer::LogFilterOptions options;

    const auto filtered_entries =
        log_analyzer::LogFilter::apply(
            read_result.entries,
            options);

    constexpr std::size_t top_errors_limit = 5;

    const auto statistics =
        log_analyzer::LogStatistics::calculate(
            filtered_entries,
            top_errors_limit);

    std::cout << "Log Analyzer CLI 0.5.0\n";

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

    if (filtered_entries.empty()) {
        std::cout
            << "No log entries match "
               "the selected filters.\n";

        return 0;
    }

    std::cout << "Levels:\n";

    print_level_count(
        statistics,
        log_analyzer::LogLevel::Debug);

    print_level_count(
        statistics,
        log_analyzer::LogLevel::Info);

    print_level_count(
        statistics,
        log_analyzer::LogLevel::Warn);

    print_level_count(
        statistics,
        log_analyzer::LogLevel::Error);

    std::cout << "Services:\n";

    for (const auto& [service, count] :
         statistics.service_counts) {
        std::cout
            << ' '
            << service
            << ": "
            << count
            << '\n';
    }

    if (statistics.duration.has_value()) {
        std::cout << "Response time, ms:\n";

        std::cout
            << " Minimum: "
            << statistics.duration->minimum_ms
            << '\n';

        std::cout
            << " Maximum: "
            << statistics.duration->maximum_ms
            << '\n';

        std::cout
            << std::fixed
            << std::setprecision(1);

        std::cout
            << " Average: "
            << statistics.duration->average_ms
            << '\n';

        std::cout
            << " Median: "
            << statistics.duration->median_ms
            << '\n';
    }

    std::cout << "Top error messages:\n";

    if (statistics.top_errors.empty()) {
        std::cout << " No ERROR messages.\n";
    } else {
        std::size_t position = 1;

        for (const auto& error :
             statistics.top_errors) {
            std::cout
                << ' '
                << position
                << ". "
                << error.message
                << " - "
                << error.count
                << '\n';

            ++position;
        }
    }

    return 0;
}