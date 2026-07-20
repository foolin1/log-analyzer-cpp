#include "log_statistics.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <chrono>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace {

[[nodiscard]] log_analyzer::LogEntry make_entry(
    const log_analyzer::LogLevel level,
    std::string service,
    std::string message,
    const std::int64_t duration_ms)
{
    return {
        .timestamp =
            std::chrono::sys_days{
                std::chrono::year{2026} /
                std::chrono::month{7} /
                std::chrono::day{9}
            },
        .level = level,
        .service = std::move(service),
        .message = std::move(message),
        .duration_ms = duration_ms
    };
}

} // namespace

TEST_CASE(
    "Statistics counts entries by level and service")
{
    using log_analyzer::LogLevel;

    const std::vector<log_analyzer::LogEntry> entries{
        make_entry(
            LogLevel::Debug,
            "api",
            "Debug message",
            10),
        make_entry(
            LogLevel::Info,
            "auth",
            "User logged in",
            20),
        make_entry(
            LogLevel::Warn,
            "api",
            "Slow request",
            30),
        make_entry(
            LogLevel::Error,
            "payments",
            "Payment failed",
            40),
        make_entry(
            LogLevel::Error,
            "payments",
            "Payment failed",
            50)
    };

    const auto statistics =
        log_analyzer::LogStatistics::calculate(
            entries,
            5);

    REQUIRE(
        statistics.level_counts.at(
            LogLevel::Debug) == 1);

    REQUIRE(
        statistics.level_counts.at(
            LogLevel::Info) == 1);

    REQUIRE(
        statistics.level_counts.at(
            LogLevel::Warn) == 1);

    REQUIRE(
        statistics.level_counts.at(
            LogLevel::Error) == 2);

    REQUIRE(
        statistics.service_counts.at("api") == 2);

    REQUIRE(
        statistics.service_counts.at("auth") == 1);

    REQUIRE(
        statistics.service_counts.at(
            "payments") == 2);
}

TEST_CASE(
    "Statistics calculates duration values for odd count")
{
    using log_analyzer::LogLevel;

    const std::vector<log_analyzer::LogEntry> entries{
        make_entry(
            LogLevel::Info,
            "api",
            "First",
            30),
        make_entry(
            LogLevel::Info,
            "api",
            "Second",
            10),
        make_entry(
            LogLevel::Info,
            "api",
            "Third",
            20)
    };

    const auto statistics =
        log_analyzer::LogStatistics::calculate(
            entries,
            5);

    REQUIRE(statistics.duration.has_value());

    REQUIRE(
        statistics.duration->minimum_ms == 10);

    REQUIRE(
        statistics.duration->maximum_ms == 30);

    REQUIRE_THAT(
        statistics.duration->average_ms,
        Catch::Matchers::WithinAbs(
            20.0,
            0.001));

    REQUIRE_THAT(
        statistics.duration->median_ms,
        Catch::Matchers::WithinAbs(
            20.0,
            0.001));
}

TEST_CASE(
    "Statistics calculates median for even count")
{
    using log_analyzer::LogLevel;

    const std::vector<log_analyzer::LogEntry> entries{
        make_entry(
            LogLevel::Info,
            "api",
            "First",
            40),
        make_entry(
            LogLevel::Info,
            "api",
            "Second",
            10),
        make_entry(
            LogLevel::Info,
            "api",
            "Third",
            30),
        make_entry(
            LogLevel::Info,
            "api",
            "Fourth",
            20)
    };

    const auto statistics =
        log_analyzer::LogStatistics::calculate(
            entries,
            5);

    REQUIRE(statistics.duration.has_value());

    REQUIRE_THAT(
        statistics.duration->average_ms,
        Catch::Matchers::WithinAbs(
            25.0,
            0.001));

    REQUIRE_THAT(
        statistics.duration->median_ms,
        Catch::Matchers::WithinAbs(
            25.0,
            0.001));
}

TEST_CASE(
    "Statistics returns top ERROR messages in correct order")
{
    using log_analyzer::LogLevel;

    const std::vector<log_analyzer::LogEntry> entries{
        make_entry(
            LogLevel::Error,
            "payments",
            "Payment failed",
            100),
        make_entry(
            LogLevel::Error,
            "payments",
            "Payment failed",
            110),
        make_entry(
            LogLevel::Error,
            "payments",
            "Payment failed",
            120),
        make_entry(
            LogLevel::Error,
            "database",
            "Database timeout",
            200),
        make_entry(
            LogLevel::Error,
            "database",
            "Database timeout",
            210),
        make_entry(
            LogLevel::Error,
            "api",
            "External API unavailable",
            300),
        make_entry(
            LogLevel::Error,
            "api",
            "Authentication failed",
            310),
        make_entry(
            LogLevel::Info,
            "payments",
            "Payment failed",
            50)
    };

    const auto statistics =
        log_analyzer::LogStatistics::calculate(
            entries,
            3);

    REQUIRE(statistics.top_errors.size() == 3);

    REQUIRE(
        statistics.top_errors[0].message ==
        "Payment failed");
    REQUIRE(statistics.top_errors[0].count == 3);

    REQUIRE(
        statistics.top_errors[1].message ==
        "Database timeout");
    REQUIRE(statistics.top_errors[1].count == 2);

    REQUIRE(
        statistics.top_errors[2].message ==
        "Authentication failed");
    REQUIRE(statistics.top_errors[2].count == 1);
}

TEST_CASE(
    "Statistics handles an empty collection")
{
    using log_analyzer::LogLevel;

    const std::vector<log_analyzer::LogEntry> entries;

    const auto statistics =
        log_analyzer::LogStatistics::calculate(
            entries,
            5);

    REQUIRE(
        statistics.level_counts.at(
            LogLevel::Debug) == 0);

    REQUIRE(
        statistics.level_counts.at(
            LogLevel::Info) == 0);

    REQUIRE(
        statistics.level_counts.at(
            LogLevel::Warn) == 0);

    REQUIRE(
        statistics.level_counts.at(
            LogLevel::Error) == 0);

    REQUIRE(statistics.service_counts.empty());
    REQUIRE_FALSE(statistics.duration.has_value());
    REQUIRE(statistics.top_errors.empty());
}