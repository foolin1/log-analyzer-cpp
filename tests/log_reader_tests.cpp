#include "log_reader.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

namespace {

class TemporaryLogFile {
public:
    explicit TemporaryLogFile(
        const std::string_view content)
        : path_{create_unique_path()}
    {
        std::ofstream output_file{
            path_,
            std::ios::binary
        };

        if (!output_file.is_open()) {
            throw std::runtime_error{
                "Failed to create temporary test file"
            };
        }

        output_file << content;

        if (!output_file.good()) {
            throw std::runtime_error{
                "Failed to write temporary test file"
            };
        }
    }

    TemporaryLogFile(const TemporaryLogFile&) = delete;

    TemporaryLogFile& operator=(
        const TemporaryLogFile&) = delete;

    TemporaryLogFile(TemporaryLogFile&&) = delete;

    TemporaryLogFile& operator=(
        TemporaryLogFile&&) = delete;

    ~TemporaryLogFile()
    {
        std::error_code error;
        std::filesystem::remove(path_, error);
    }

    [[nodiscard]] const std::filesystem::path&
    path() const noexcept
    {
        return path_;
    }

private:
    [[nodiscard]] static std::filesystem::path
    create_unique_path()
    {
        static std::uint64_t file_number = 0;
        ++file_number;

        return std::filesystem::temp_directory_path() /
               (
                   "log-analyzer-reader-test-" +
                   std::to_string(file_number) +
                   ".log"
               );
    }

    std::filesystem::path path_;
};

[[nodiscard]] std::filesystem::path
create_missing_file_path()
{
    static std::uint64_t file_number = 0;
    ++file_number;

    const auto path =
        std::filesystem::temp_directory_path() /
        (
            "log-analyzer-missing-test-" +
            std::to_string(file_number) +
            ".log"
        );

    std::error_code error;
    std::filesystem::remove(path, error);

    return path;
}

} // namespace

TEST_CASE(
    "Reader collects valid entries and counts invalid lines")
{
    const TemporaryLogFile file{
        "2026-07-09T10:15:30 INFO auth "
        "User logged in 125\n"
        "2026-07-09T10:16:04 TRACE payments "
        "Unknown level 840\n"
        "\n"
        "2026-07-09T10:17:11 ERROR api "
        "Request failed 1250\n"
    };

    const auto result =
        log_analyzer::LogReader::read(file.path());

    REQUIRE(result.success());
    REQUIRE(result.total_lines == 4);
    REQUIRE(result.entries.size() == 2);
    REQUIRE(result.invalid_lines == 2);

    REQUIRE(result.entries[0].service == "auth");
    REQUIRE(
        result.entries[0].level ==
        log_analyzer::LogLevel::Info);
    REQUIRE(
        result.entries[0].message ==
        "User logged in");
    REQUIRE(result.entries[0].duration_ms == 125);

    REQUIRE(result.entries[1].service == "api");
    REQUIRE(
        result.entries[1].level ==
        log_analyzer::LogLevel::Error);
    REQUIRE(
        result.entries[1].message ==
        "Request failed");
    REQUIRE(result.entries[1].duration_ms == 1250);
}

TEST_CASE("Reader handles an empty log file")
{
    const TemporaryLogFile file{""};

    const auto result =
        log_analyzer::LogReader::read(file.path());

    REQUIRE(result.success());
    REQUIRE(result.total_lines == 0);
    REQUIRE(result.entries.empty());
    REQUIRE(result.invalid_lines == 0);
}

TEST_CASE("Reader reports a missing input file")
{
    const auto missing_path =
        create_missing_file_path();

    const auto result =
        log_analyzer::LogReader::read(missing_path);

    REQUIRE_FALSE(result.success());
    REQUIRE(result.entries.empty());
    REQUIRE(result.total_lines == 0);
    REQUIRE(result.invalid_lines == 0);

    REQUIRE(
        result.error.find(
            "input file does not exist") !=
        std::string::npos);
}