#include "csv_exporter.hpp"

#include "log_level.hpp"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>

namespace log_analyzer {

namespace {

[[nodiscard]] bool requires_quotes(
    const std::string_view value)
{
    return value.find(',') != std::string_view::npos ||
           value.find('"') != std::string_view::npos ||
           value.find('\n') != std::string_view::npos ||
           value.find('\r') != std::string_view::npos;
}

[[nodiscard]] std::string escape_csv_field(
    const std::string_view value)
{
    if (!requires_quotes(value)) {
        return std::string{value};
    }

    std::string escaped;
    escaped.reserve(value.size() + 2);

    escaped.push_back('"');

    for (const char character : value) {
        if (character == '"') {
            escaped.push_back('"');
        }

        escaped.push_back(character);
    }

    escaped.push_back('"');

    return escaped;
}

[[nodiscard]] std::string format_timestamp(
    const std::chrono::sys_seconds timestamp)
{
    const auto day_point =
        std::chrono::floor<std::chrono::days>(
            timestamp);

    const std::chrono::year_month_day date{
        day_point
    };

    const std::chrono::hh_mm_ss<
        std::chrono::seconds> time_of_day{
            timestamp - day_point
        };

    std::ostringstream output;

    output
        << std::setfill('0')
        << std::setw(4)
        << static_cast<int>(date.year())
        << '-'
        << std::setw(2)
        << static_cast<unsigned int>(date.month())
        << '-'
        << std::setw(2)
        << static_cast<unsigned int>(date.day())
        << 'T'
        << std::setw(2)
        << time_of_day.hours().count()
        << ':'
        << std::setw(2)
        << time_of_day.minutes().count()
        << ':'
        << std::setw(2)
        << time_of_day.seconds().count();

    return output.str();
}

} // namespace

CsvExportResult CsvExporter::export_entries(
    const std::vector<LogEntry>& entries,
    const std::filesystem::path& output_path)
{
    std::ofstream output_file{
        output_path,
        std::ios::binary |
            std::ios::trunc
    };

    if (!output_file.is_open()) {
        return {
            .rows_written = 0,
            .error =
                "cannot create or overwrite output file: " +
                output_path.string()
        };
    }

    output_file
        << "timestamp,level,service,message,"
           "duration_ms\n";

    for (const auto& entry : entries) {
        output_file
            << escape_csv_field(
                   format_timestamp(entry.timestamp))
            << ','
            << escape_csv_field(
                   to_string(entry.level))
            << ','
            << escape_csv_field(entry.service)
            << ','
            << escape_csv_field(entry.message)
            << ','
            << entry.duration_ms
            << '\n';
    }

    output_file.flush();

    if (!output_file.good()) {
        return {
            .rows_written = 0,
            .error =
                "failed to write output file: " +
                output_path.string()
        };
    }

    return {
        .rows_written = entries.size(),
        .error = {}
    };
}

} // namespace log_analyzer