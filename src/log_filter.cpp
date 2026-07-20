#include "log_filter.hpp"

#include <charconv>
#include <chrono>
#include <cstddef>
#include <string_view>
#include <system_error>
#include <vector>

namespace log_analyzer {

namespace {

[[nodiscard]] bool parse_fixed_integer(
    const std::string_view value,
    const std::size_t position,
    const std::size_t length,
    int& result)
{
    const std::string_view part =
        value.substr(position, length);

    const char* const begin = part.data();
    const char* const end = begin + part.size();

    const auto [pointer, error] =
        std::from_chars(begin, end, result);

    return error == std::errc{} &&
           pointer == end;
}

} // namespace

std::optional<std::chrono::sys_days>
parse_filter_date(const std::string_view value)
{
    if (value.size() != 10 ||
        value[4] != '-' ||
        value[7] != '-') {
        return std::nullopt;
    }

    int year_value = 0;
    int month_value = 0;
    int day_value = 0;

    if (!parse_fixed_integer(
            value,
            0,
            4,
            year_value) ||
        !parse_fixed_integer(
            value,
            5,
            2,
            month_value) ||
        !parse_fixed_integer(
            value,
            8,
            2,
            day_value)) {
        return std::nullopt;
    }

    if (year_value < 1 ||
        year_value > 9999) {
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

    return std::chrono::sys_days{date};
}

std::vector<LogEntry> LogFilter::apply(
    const std::vector<LogEntry>& entries,
    const LogFilterOptions& options)
{
    std::vector<LogEntry> filtered_entries;
    filtered_entries.reserve(entries.size());

    for (const auto& entry : entries) {
        if (matches(entry, options)) {
            filtered_entries.push_back(entry);
        }
    }

    return filtered_entries;
}

bool LogFilter::matches(
    const LogEntry& entry,
    const LogFilterOptions& options)
{
    if (options.level.has_value() &&
        entry.level != *options.level) {
        return false;
    }

    if (options.service.has_value() &&
        entry.service != *options.service) {
        return false;
    }

    const auto entry_date =
        std::chrono::floor<std::chrono::days>(
            entry.timestamp);

    if (options.from_date.has_value() &&
        entry_date < *options.from_date) {
        return false;
    }

    if (options.to_date.has_value() &&
        entry_date > *options.to_date) {
        return false;
    }

    return true;
}

} // namespace log_analyzer