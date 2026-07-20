#include "command_line_options.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string_view>
#include <vector>

namespace {

[[nodiscard]] log_analyzer::CommandLineParseResult
parse(
    const std::initializer_list<std::string_view>
        arguments)
{
    return
        log_analyzer::CommandLineOptionsParser::parse(
            std::vector<std::string_view>{
                arguments
            });
}

} // namespace

TEST_CASE("CLI parses analyze command and filters")
{
    const auto result = parse({
        "analyze",
        "--input",
        "samples/application.log",
        "--level",
        "error",
        "--service",
        "payments",
        "--from",
        "2026-07-01",
        "--to",
        "2026-07-31",
        "--top-errors",
        "3"
    });

    REQUIRE(result.success());
    REQUIRE(result.options.has_value());

    REQUIRE(
        result.options->command ==
        log_analyzer::Command::Analyze);

    REQUIRE(
        result.options->input_path ==
        "samples/application.log");

    REQUIRE(
        result.options->filters.level ==
        log_analyzer::LogLevel::Error);

    REQUIRE(
        result.options->filters.service ==
        "payments");

    REQUIRE(
        result.options->filters.from_date
            .has_value());

    REQUIRE(
        result.options->filters.to_date
            .has_value());

    REQUIRE(
        result.options->top_errors_limit == 3);
}

TEST_CASE("CLI parses export command")
{
    const auto result = parse({
        "export",
        "--input",
        "samples/application.log",
        "--output",
        "errors.csv",
        "--level",
        "ERROR"
    });

    REQUIRE(result.success());
    REQUIRE(result.options.has_value());

    REQUIRE(
        result.options->command ==
        log_analyzer::Command::Export);

    REQUIRE(
        result.options->output_path.has_value());

    REQUIRE(
        *result.options->output_path ==
        "errors.csv");
}

TEST_CASE("CLI supports help without input file")
{
    const auto result = parse({"--help"});

    REQUIRE(result.success());
    REQUIRE(result.show_help);
    REQUIRE_FALSE(result.show_version);
    REQUIRE_FALSE(result.options.has_value());
}

TEST_CASE("CLI supports version without input file")
{
    const auto result = parse({"--version"});

    REQUIRE(result.success());
    REQUIRE(result.show_version);
    REQUIRE_FALSE(result.show_help);
    REQUIRE_FALSE(result.options.has_value());
}

TEST_CASE("CLI requires input file")
{
    const auto result = parse({"analyze"});

    REQUIRE_FALSE(result.success());
    REQUIRE(result.error == "--input is required");
}

TEST_CASE("CLI requires output file for export")
{
    const auto result = parse({
        "export",
        "--input",
        "samples/application.log"
    });

    REQUIRE_FALSE(result.success());

    REQUIRE(
        result.error ==
        "--output is required for "
        "the export command");
}

TEST_CASE("CLI rejects reversed date range")
{
    const auto result = parse({
        "analyze",
        "--input",
        "samples/application.log",
        "--from",
        "2026-07-31",
        "--to",
        "2026-07-01"
    });

    REQUIRE_FALSE(result.success());

    REQUIRE(
        result.error ==
        "--from date must not be later "
        "than --to date");
}

TEST_CASE("CLI rejects invalid top errors value")
{
    const auto result = parse({
        "analyze",
        "--input",
        "samples/application.log",
        "--top-errors",
        "0"
    });

    REQUIRE_FALSE(result.success());

    REQUIRE(
        result.error ==
        "--top-errors must be "
        "a positive integer");
}

TEST_CASE("CLI rejects unknown option")
{
    const auto result = parse({
        "analyze",
        "--input",
        "samples/application.log",
        "--unknown",
        "value"
    });

    REQUIRE_FALSE(result.success());

    REQUIRE(
        result.error ==
        "unknown option: --unknown");
}