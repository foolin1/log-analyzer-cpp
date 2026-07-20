#pragma once

#include "log_entry.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace log_analyzer {

struct DurationStatistics {
    std::int64_t minimum_ms{};
    std::int64_t maximum_ms{};
    double average_ms{};
    double median_ms{};
};

struct ErrorMessageStatistics {
    std::string message;
    std::size_t count{};
};

struct LogStatisticsResult {
    std::map<LogLevel, std::size_t> level_counts;
    std::map<std::string, std::size_t> service_counts;
    std::optional<DurationStatistics> duration;
    std::vector<ErrorMessageStatistics> top_errors;
};

class LogStatistics {
public:
    [[nodiscard]] static LogStatisticsResult calculate(
        const std::vector<LogEntry>& entries,
        std::size_t top_errors_limit);
};

} // namespace log_analyzer