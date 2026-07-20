#include "log_parser.hpp"

#include <charconv>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace log_analyzer {

namespace {

[[nodiscard]] bool parse_fixed_integer(
    const std::string_view value,
    const std::size_t position,
    const std::size_t length,
    int& result)
{
    const std::string_view part = value.substr(position, length);

    const char* const begin = part.data();
    const char* const end = begin + part.size();

    const auto [pointer, error] =
        std::from_chars(begin, end, result);

    return error == std::errc{} && pointer == end;
}

[[nodiscard]] std::optional<std::chrono::sys_seconds>
parse_timestamp(const std::string_view value)
{
    if (value.size() != 19 ||
        value[4] != '-' ||
        value[7] != '-' ||
        value[10] != 'T' ||
        value[13] != ':' ||
        value[16] != ':') {
        return std::nullopt;
    }

    int year_value = 0;
    int month_value = 0;
    int day_value = 0;
    int hour_value = 0;
    int minute_value = 0;
    int second_value = 0;

    if (!parse_fixed_integer(value, 0, 4, year_value) ||
        !parse_fixed_integer(value, 5, 2, month_value) ||
        !parse_fixed_integer(value, 8, 2, day_value) ||
        !parse_fixed_integer(value, 11, 2, hour_value) ||
        !parse_fixed_integer(value, 14, 2, minute_value) ||
        !parse_fixed_integer(value, 17, 2, second_value)) {
        return std::nullopt;
    }

    if (year_value < 1 ||
        year_value > 9999 ||
        hour_value < 0 ||
        hour_value > 23 ||
        minute_value < 0 ||
        minute_value > 59 ||
        second_value < 0 ||
        second_value > 59) {
        return std::nullopt;
    }

    const std::chrono::year_month_day date{
        std::chrono::year{year_value},
        std::chrono::month{
            static_cast<unsigned int>(month_value)},
        std::chrono::day{
            static_cast<unsigned int>(day_value)}
    };

    if (!date.ok()) {
        return std::nullopt;
    }

    return std::chrono::sys_days{date} +
           std::chrono::hours{hour_value} +
           std::chrono::minutes{minute_value} +
           std::chrono::seconds{second_value};
}

[[nodiscard]] std::optional<std::int64_t>
parse_duration(const std::string_view value)
{
    std::int64_t duration = 0;

    const char* const begin = value.data();
    const char* const end = begin + value.size();

    const auto [pointer, error] =
        std::from_chars(begin, end, duration);

    if (error != std::errc{} ||
        pointer != end ||
        duration < 0) {
        return std::nullopt;
    }

    return duration;
}

} // namespace

LogParseResult LogParser::parse(const std::string_view line)
{
    std::istringstream stream{std::string{line}};
    std::vector<std::string> tokens;
    std::string token;

    while (stream >> token) {
        tokens.push_back(token);
    }

    if (tokens.size() < 4) {
        return {
            .entry = std::nullopt,
            .error = "missing required fields"
        };
    }

    if (tokens.size() == 4) {
        return {
            .entry = std::nullopt,
            .error = "message must not be empty"
        };
    }

    const auto timestamp = parse_timestamp(tokens[0]);

    if (!timestamp.has_value()) {
        return {
            .entry = std::nullopt,
            .error = "invalid timestamp"
        };
    }

    const auto level = log_level_from_string(tokens[1]);

    if (!level.has_value()) {
        return {
            .entry = std::nullopt,
            .error = "unknown log level"
        };
    }

    if (tokens[2].empty()) {
        return {
            .entry = std::nullopt,
            .error = "service must not be empty"
        };
    }

    const auto duration = parse_duration(tokens.back());

    if (!duration.has_value()) {
        return {
            .entry = std::nullopt,
            .error = "invalid duration"
        };
    }

    std::string message;

    for (std::size_t index = 3;
         index + 1 < tokens.size();
         ++index) {
        if (!message.empty()) {
            message += ' ';
        }

        message += tokens[index];
    }

    if (message.empty()) {
        return {
            .entry = std::nullopt,
            .error = "message must not be empty"
        };
    }

    return {
        .entry = LogEntry{
            .timestamp = *timestamp,
            .level = *level,
            .service = tokens[2],
            .message = std::move(message),
            .duration_ms = *duration
        },
        .error = {}
    };
}

} // namespace log_analyzer