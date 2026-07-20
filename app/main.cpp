#include "command_line_options.hpp"
#include "log_filter.hpp"
#include "log_level.hpp"
#include "log_reader.hpp"
#include "log_statistics.hpp"

#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string_view>
#include <vector>

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

void print_report(
    const log_analyzer::CommandLineOptions& options,
    const log_analyzer::LogReadResult& read_result,
    const std::vector<log_analyzer::LogEntry>& entries)
{
    const auto statistics =
        log_analyzer::LogStatistics::calculate(
            entries,
            options.top_errors_limit);

    std::cout
        << "Log file: "
        << options.input_path.string()
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
        << entries.size()
        << '\n';

    if (entries.empty()) {
        std::cout
            << "No log entries match "
               "the selected filters.\n";

        return;
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
        return;
    }

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

} // namespace

int main(const int argc, const char* const argv[])
{
    std::vector<std::string_view> arguments;
    arguments.reserve(
        argc > 1
            ? static_cast<std::size_t>(argc - 1)
            : 0);

    for (int index = 1; index < argc; ++index) {
        arguments.emplace_back(argv[index]);
    }

    const auto parse_result =
        log_analyzer::CommandLineOptionsParser::parse(
            arguments);

    if (!parse_result.success()) {
        std::cerr
            << "Error: "
            << parse_result.error
            << '\n';

        return 1;
    }

    if (parse_result.show_help) {
        std::cout << log_analyzer::help_text();
        return 0;
    }

    if (parse_result.show_version) {
        std::cout << log_analyzer::version_text();
        return 0;
    }

    const auto& options =
        *parse_result.options;

    const auto read_result =
        log_analyzer::LogReader::read(
            options.input_path);

    if (!read_result.success()) {
        std::cerr
            << "Error: "
            << read_result.error
            << '\n';

        return 2;
    }

    const auto filtered_entries =
        log_analyzer::LogFilter::apply(
            read_result.entries,
            options.filters);

    if (options.command ==
        log_analyzer::Command::Export) {
        std::cerr
            << "Error: CSV export is not available "
               "in version 0.6.0\n";

        return 3;
    }

    print_report(
        options,
        read_result,
        filtered_entries);

    return 0;
}