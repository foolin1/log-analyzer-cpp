#pragma once

#include <optional>
#include <string_view>

namespace log_analyzer {

enum class LogLevel {
    Debug,
    Info,
    Warn,
    Error
};

[[nodiscard]] std::optional<LogLevel> log_level_from_string(
    std::string_view value);

[[nodiscard]] std::string_view to_string(LogLevel level) noexcept;

} // namespace log_analyzer