#include "csv_exporter.hpp"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace {

class TemporaryCsvFile {
public:
    TemporaryCsvFile()
        : path_{create_unique_path()}
    {
    }

    TemporaryCsvFile(
        const TemporaryCsvFile&) = delete;

    TemporaryCsvFile& operator=(
        const TemporaryCsvFile&) = delete;

    TemporaryCsvFile(
        TemporaryCsvFile&&) = delete;

    TemporaryCsvFile& operator=(
        TemporaryCsvFile&&) = delete;

    ~TemporaryCsvFile()
    {
        std::error_code error;
        std::filesystem::remove(path_, error);
    }

    [[nodiscard]] const std::filesystem::path&
    path() const noexcept
    {
        return path_;
    }

    [[nodiscard]] std::string read() const
    {
        std::ifstream input_file{
            path_,
            std::ios::binary
        };

        if (!input_file.is_open()) {
            throw std::runtime_error{
                "Failed to open temporary CSV file"
            };
        }

        return {
            std::istreambuf_iterator<char>{
                input_file
            },
            std::istreambuf_iterator<char>{}
        };
    }

private:
    [[nodiscard]] static std::filesystem::path
    create_unique_path()
    {
        static std::uint64_t file_number = 0;
        ++file_number;

        return
            std::filesystem::temp_directory_path() /
            (
                "log-analyzer-export-test-" +
                std::to_string(file_number) +
                ".csv"
            );
    }

    std::filesystem::path path_;
};

[[nodiscard]] log_analyzer::LogEntry make_entry(
    const log_analyzer::LogLevel level,
    std::string service,
    std::string message,
    const std::int64_t duration_ms)
{
    const std::chrono::year_month_day date{
        std::chrono::year{2026},
        std::chrono::month{7},
        std::chrono::day{9}
    };

    return {
        .timestamp =
            std::chrono::sys_days{date} +
            std::chrono::hours{10} +
            std::chrono::minutes{15} +
            std::chrono::seconds{30},
        .level = level,
        .service = std::move(service),
        .message = std::move(message),
        .duration_ms = duration_ms
    };
}

[[nodiscard]] std::filesystem::path
create_unavailable_output_path()
{
    static std::uint64_t directory_number = 0;
    ++directory_number;

    const auto directory =
        std::filesystem::temp_directory_path() /
        (
            "log-analyzer-missing-directory-" +
            std::to_string(directory_number)
        );

    std::error_code error;
    std::filesystem::remove_all(
        directory,
        error);

    return directory / "output.csv";
}

} // namespace

TEST_CASE("CSV exporter writes header and regular values")
{
    const TemporaryCsvFile file;

    const std::vector<log_analyzer::LogEntry> entries{
        make_entry(
            log_analyzer::LogLevel::Info,
            "auth",
            "User logged in",
            125)
    };

    const auto result =
        log_analyzer::CsvExporter::export_entries(
            entries,
            file.path());

    REQUIRE(result.success());
    REQUIRE(result.rows_written == 1);

    REQUIRE(
        file.read() ==
        "timestamp,level,service,message,"
        "duration_ms\n"
        "2026-07-09T10:15:30,INFO,auth,"
        "User logged in,125\n");
}

TEST_CASE("CSV exporter escapes commas and quotes")
{
    const TemporaryCsvFile file;

    const std::vector<log_analyzer::LogEntry> entries{
        make_entry(
            log_analyzer::LogLevel::Error,
            "payments",
            "Payment failed, retry \"scheduled\"",
            840)
    };

    const auto result =
        log_analyzer::CsvExporter::export_entries(
            entries,
            file.path());

    REQUIRE(result.success());
    REQUIRE(result.rows_written == 1);

    REQUIRE(
        file.read() ==
        "timestamp,level,service,message,"
        "duration_ms\n"
        "2026-07-09T10:15:30,ERROR,payments,"
        "\"Payment failed, retry "
        "\"\"scheduled\"\"\",840\n");
}

TEST_CASE("CSV exporter writes only header for empty input")
{
    const TemporaryCsvFile file;

    const std::vector<log_analyzer::LogEntry> entries;

    const auto result =
        log_analyzer::CsvExporter::export_entries(
            entries,
            file.path());

    REQUIRE(result.success());
    REQUIRE(result.rows_written == 0);

    REQUIRE(
        file.read() ==
        "timestamp,level,service,message,"
        "duration_ms\n");
}

TEST_CASE("CSV exporter reports unavailable output path")
{
    const std::vector<log_analyzer::LogEntry> entries{
        make_entry(
            log_analyzer::LogLevel::Info,
            "api",
            "Request completed",
            100)
    };

    const auto result =
        log_analyzer::CsvExporter::export_entries(
            entries,
            create_unavailable_output_path());

    REQUIRE_FALSE(result.success());
    REQUIRE(result.rows_written == 0);

    REQUIRE(
        result.error.find(
            "cannot create or overwrite output file") !=
        std::string::npos);
}