#include "log_statistics.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace log_analyzer {

namespace {

[[nodiscard]] std::map<LogLevel, std::size_t>
create_initial_level_counts()
{
    return {
        {LogLevel::Debug, 0},
        {LogLevel::Info, 0},
        {LogLevel::Warn, 0},
        {LogLevel::Error, 0}
    };
}

[[nodiscard]] DurationStatistics calculate_duration_statistics(
    std::vector<std::int64_t> durations)
{
    std::sort(
        durations.begin(),
        durations.end());

    const auto duration_sum =
        std::accumulate(
            durations.begin(),
            durations.end(),
            0.0L);

    const auto duration_count =
        static_cast<long double>(durations.size());

    double median = 0.0;

    const std::size_t middle_index =
        durations.size() / 2;

    if (durations.size() % 2 == 0) {
        const auto left_value =
            static_cast<long double>(
                durations[middle_index - 1]);

        const auto right_value =
            static_cast<long double>(
                durations[middle_index]);

        median = static_cast<double>(
            (left_value + right_value) / 2.0L);
    } else {
        median = static_cast<double>(
            durations[middle_index]);
    }

    return {
        .minimum_ms = durations.front(),
        .maximum_ms = durations.back(),
        .average_ms = static_cast<double>(
            duration_sum / duration_count),
        .median_ms = median
    };
}

[[nodiscard]] std::vector<ErrorMessageStatistics>
create_top_errors(
    const std::unordered_map<std::string, std::size_t>&
        error_counts,
    const std::size_t limit)
{
    std::vector<ErrorMessageStatistics> top_errors;
    top_errors.reserve(error_counts.size());

    for (const auto& [message, count] : error_counts) {
        top_errors.push_back({
            .message = message,
            .count = count
        });
    }

    std::sort(
        top_errors.begin(),
        top_errors.end(),
        [](const ErrorMessageStatistics& left,
           const ErrorMessageStatistics& right) {
            if (left.count != right.count) {
                return left.count > right.count;
            }

            return left.message < right.message;
        });

    if (top_errors.size() > limit) {
        top_errors.resize(limit);
    }

    return top_errors;
}

} // namespace

LogStatisticsResult LogStatistics::calculate(
    const std::vector<LogEntry>& entries,
    const std::size_t top_errors_limit)
{
    LogStatisticsResult result;
    result.level_counts =
        create_initial_level_counts();

    std::vector<std::int64_t> durations;
    durations.reserve(entries.size());

    std::unordered_map<std::string, std::size_t>
        error_counts;

    for (const auto& entry : entries) {
        ++result.level_counts[entry.level];
        ++result.service_counts[entry.service];

        durations.push_back(entry.duration_ms);

        if (entry.level == LogLevel::Error) {
            ++error_counts[entry.message];
        }
    }

    if (!durations.empty()) {
        result.duration =
            calculate_duration_statistics(
                std::move(durations));
    }

    result.top_errors =
        create_top_errors(
            error_counts,
            top_errors_limit);

    return result;
}

} // namespace log_analyzer