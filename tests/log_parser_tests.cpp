#include "log_parser.hpp"

#include <catch2/catch_test_macros.hpp>

#include <chrono>

TEST_CASE("Parser reads a valid log entry")
{
    const auto result =
        log_analyzer::LogParser::parse(
            "2026-07-09T10:15:30 INFO auth "
            "User logged in 125");

    REQUIRE(result.success());
    REQUIRE(result.entry.has_value());

    const auto& entry = *result.entry;

    const std::chrono::year_month_day date{
        std::chrono::year{2026},
        std::chrono::month{7},
        std::chrono::day{9}
    };

    const auto expected_timestamp =
        std::chrono::sys_days{date} +
        std::chrono::hours{10} +
        std::chrono::minutes{15} +
        std::chrono::seconds{30};

    REQUIRE(entry.timestamp == expected_timestamp);
    REQUIRE(entry.level == log_analyzer::LogLevel::Info);
    REQUIRE(entry.service == "auth");
    REQUIRE(entry.message == "User logged in");
    REQUIRE(entry.duration_ms == 125);
}

TEST_CASE("Parser joins a multi-word message")
{
    const auto result =
        log_analyzer::LogParser::parse(
            "2026-07-09T10:16:04 ERROR payments "
            "Payment failed after retry 840");

    REQUIRE(result.success());
    REQUIRE(
        result.entry->message ==
        "Payment failed after retry");
}

TEST_CASE("Parser rejects a line with missing fields")
{
    const auto result =
        log_analyzer::LogParser::parse(
            "2026-07-09T10:15:30 INFO auth");

    REQUIRE_FALSE(result.success());
    REQUIRE(result.error == "missing required fields");
}

TEST_CASE("Parser rejects an unknown log level")
{
    const auto result =
        log_analyzer::LogParser::parse(
            "2026-07-09T10:15:30 TRACE auth "
            "Request completed 125");

    REQUIRE_FALSE(result.success());
    REQUIRE(result.error == "unknown log level");
}

TEST_CASE("Parser rejects an invalid timestamp")
{
    const auto result =
        log_analyzer::LogParser::parse(
            "2026-02-30T10:15:30 INFO auth "
            "Request completed 125");

    REQUIRE_FALSE(result.success());
    REQUIRE(result.error == "invalid timestamp");
}

TEST_CASE("Parser rejects a non-numeric duration")
{
    const auto result =
        log_analyzer::LogParser::parse(
            "2026-07-09T10:15:30 INFO auth "
            "Request completed abc");

    REQUIRE_FALSE(result.success());
    REQUIRE(result.error == "invalid duration");
}

TEST_CASE("Parser rejects a negative duration")
{
    const auto result =
        log_analyzer::LogParser::parse(
            "2026-07-09T10:15:30 INFO auth "
            "Request completed -10");

    REQUIRE_FALSE(result.success());
    REQUIRE(result.error == "invalid duration");
}

TEST_CASE("Parser rejects an empty message")
{
    const auto result =
        log_analyzer::LogParser::parse(
            "2026-07-09T10:15:30 INFO auth 125");

    REQUIRE_FALSE(result.success());
    REQUIRE(result.error == "message must not be empty");
}