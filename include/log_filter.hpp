#pragma once

#include "log_entry.hpp"

#include <chrono>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace log_analyzer {

struct LogFilterOptions {
    std::optional<LogLevel> level;
    std::optional<std::string> service;
    std::optional<std::chrono::sys_days> from_date;
    std::optional<std::chrono::sys_days> to_date;
};

[[nodiscard]] std::optional<std::chrono::sys_days>
parse_filter_date(std::string_view value);

class LogFilter {
public:
    [[nodiscard]] static std::vector<LogEntry> apply(
        const std::vector<LogEntry>& entries,
        const LogFilterOptions& options);

private:
    [[nodiscard]] static bool matches(
        const LogEntry& entry,
        const LogFilterOptions& options);
};

} // namespace log_analyzer