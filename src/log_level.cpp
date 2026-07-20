#include "log_level.hpp"

#include <algorithm>
#include <cctype>
#include <string>

namespace log_analyzer {

std::optional<LogLevel> log_level_from_string(
    const std::string_view value)
{
    std::string normalized{value};

    std::transform(
        normalized.begin(),
        normalized.end(),
        normalized.begin(),
        [](const unsigned char character) {
            return static_cast<char>(std::toupper(character));
        });

    if (normalized == "DEBUG") {
        return LogLevel::Debug;
    }

    if (normalized == "INFO") {
        return LogLevel::Info;
    }

    if (normalized == "WARN") {
        return LogLevel::Warn;
    }

    if (normalized == "ERROR") {
        return LogLevel::Error;
    }

    return std::nullopt;
}

std::string_view to_string(const LogLevel level) noexcept
{
    switch (level) {
    case LogLevel::Debug:
        return "DEBUG";

    case LogLevel::Info:
        return "INFO";

    case LogLevel::Warn:
        return "WARN";

    case LogLevel::Error:
        return "ERROR";
    }

    return "UNKNOWN";
}

} // namespace log_analyzer