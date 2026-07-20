#pragma once

#include "log_entry.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace log_analyzer {

struct LogParseResult {
    std::optional<LogEntry> entry;
    std::string error;

    [[nodiscard]] bool success() const noexcept
    {
        return entry.has_value();
    }
};

class LogParser {
public:
    [[nodiscard]] static LogParseResult parse(std::string_view line);
};

} // namespace log_analyzer