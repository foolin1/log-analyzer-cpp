#include "log_filter.hpp"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace {

[[nodiscard]] log_analyzer::LogEntry make_entry(
    const int year,
    const unsigned int month,
    const unsigned int day,
    const int hour,
    const log_analyzer::LogLevel level,
    std::string service,
    std::string message,
    const std::int64_t duration_ms)
{
    const std::chrono::year_month_day date{
        std::chrono::year{year},
        std::chrono::month{month},
        std::chrono::day{day}
    };

    return {
        .timestamp =
            std::chrono::sys_days{date} +
            std::chrono::hours{hour},
        .level = level,
        .service = std::move(service),
        .message = std::move(message),
        .duration_ms = duration_ms
    };
}

[[nodiscard]] std::vector<log_analyzer::LogEntry>
create_entries()
{
    using log_analyzer::LogLevel;

    return {
        make_entry(
            2026,
            7,
            8,
            9,
            LogLevel::Info,
            "auth",
            "User logged in",
            125),
        make_entry(
            2026,
            7,
            9,
            10,
            LogLevel::Error,
            "payments",
            "Payment failed",
            840),
        make_entry(
            2026,
            7,
            9,
            11,
            LogLevel::Warn,
            "api",
            "Slow request",
            1250),
        make_entry(
            2026,
            7,
            10,
            8,
            LogLevel::Error,
            "payments",
            "Payment failed after retry",
            910),
        make_entry(
            2026,
            7,
            10,
            9,
            LogLevel::Info,
            "auth",
            "User logged out",
            75),
        make_entry(
            2026,
            7,
            11,
            12,
            LogLevel::Error,
            "Payments",
            "Case-sensitive service",
            500)
    };
}

} // namespace

TEST_CASE("Filter selects entries by log level")
{
    const auto entries = create_entries();

    log_analyzer::LogFilterOptions options;
    options.level = log_analyzer::LogLevel::Error;

    const auto filtered =
        log_analyzer::LogFilter::apply(
            entries,
            options);

    REQUIRE(filtered.size() == 3);

    for (const auto& entry : filtered) {
        REQUIRE(
            entry.level ==
            log_analyzer::LogLevel::Error);
    }
}

TEST_CASE(
    "Filter selects service using exact case-sensitive match")
{
    const auto entries = create_entries();

    log_analyzer::LogFilterOptions options;
    options.service = "payments";

    const auto filtered =
        log_analyzer::LogFilter::apply(
            entries,
            options);

    REQUIRE(filtered.size() == 2);
    REQUIRE(filtered[0].service == "payments");
    REQUIRE(filtered[1].service == "payments");
}

TEST_CASE(
    "Filter includes entries on and after the from date")
{
    const auto entries = create_entries();

    log_analyzer::LogFilterOptions options;
    options.from_date =
        log_analyzer::parse_filter_date(
            "2026-07-10");

    REQUIRE(options.from_date.has_value());

    const auto filtered =
        log_analyzer::LogFilter::apply(
            entries,
            options);

    REQUIRE(filtered.size() == 3);

    REQUIRE(
        filtered[0].message ==
        "Payment failed after retry");
}

TEST_CASE(
    "Filter includes entries on and before the to date")
{
    const auto entries = create_entries();

    log_analyzer::LogFilterOptions options;
    options.to_date =
        log_analyzer::parse_filter_date(
            "2026-07-09");

    REQUIRE(options.to_date.has_value());

    const auto filtered =
        log_analyzer::LogFilter::apply(
            entries,
            options);

    REQUIRE(filtered.size() == 3);
    REQUIRE(
        filtered.back().message ==
        "Slow request");
}

TEST_CASE(
    "Filter applies multiple conditions using logical AND")
{
    const auto entries = create_entries();

    log_analyzer::LogFilterOptions options;
    options.level =
        log_analyzer::LogLevel::Error;
    options.service = "payments";
    options.from_date =
        log_analyzer::parse_filter_date(
            "2026-07-10");
    options.to_date =
        log_analyzer::parse_filter_date(
            "2026-07-10");

    const auto filtered =
        log_analyzer::LogFilter::apply(
            entries,
            options);

    REQUIRE(filtered.size() == 1);

    REQUIRE(
        filtered[0].message ==
        "Payment failed after retry");
    REQUIRE(filtered[0].duration_ms == 910);
}

TEST_CASE(
    "Filter returns an empty collection when nothing matches")
{
    const auto entries = create_entries();

    log_analyzer::LogFilterOptions options;
    options.service = "missing-service";

    const auto filtered =
        log_analyzer::LogFilter::apply(
            entries,
            options);

    REQUIRE(filtered.empty());
}

TEST_CASE(
    "Date parser accepts valid dates and rejects invalid dates")
{
    const auto valid_date =
        log_analyzer::parse_filter_date(
            "2026-07-09");

    REQUIRE(valid_date.has_value());

    REQUIRE_FALSE(
        log_analyzer::parse_filter_date(
            "2026-02-30")
            .has_value());

    REQUIRE_FALSE(
        log_analyzer::parse_filter_date(
            "2026/07/09")
            .has_value());

    REQUIRE_FALSE(
        log_analyzer::parse_filter_date(
            "09-07-2026")
            .has_value());

    REQUIRE_FALSE(
        log_analyzer::parse_filter_date(
            "invalid")
            .has_value());
}