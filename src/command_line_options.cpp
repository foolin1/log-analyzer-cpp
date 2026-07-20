#include "command_line_options.hpp"

#include "log_level.hpp"

#include <algorithm>
#include <charconv>
#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace log_analyzer {

namespace {

[[nodiscard]] bool contains_argument(
    const std::vector<std::string_view>& arguments,
    const std::string_view expected)
{
    return std::find(
               arguments.begin(),
               arguments.end(),
               expected) != arguments.end();
}

[[nodiscard]] bool is_option(
    const std::string_view value)
{
    return value.starts_with("--");
}

[[nodiscard]] std::optional<std::size_t>
parse_positive_number(const std::string_view value)
{
    if (value.empty()) {
        return std::nullopt;
    }

    std::size_t result = 0;

    const char* const begin = value.data();
    const char* const end = begin + value.size();

    const auto [pointer, error] =
        std::from_chars(begin, end, result);

    if (error != std::errc{} ||
        pointer != end ||
        result == 0) {
        return std::nullopt;
    }

    return result;
}

[[nodiscard]] CommandLineParseResult
make_error(std::string message)
{
    return {
        .options = std::nullopt,
        .show_help = false,
        .show_version = false,
        .error = std::move(message)
    };
}

} // namespace

CommandLineParseResult CommandLineOptionsParser::parse(
    const std::vector<std::string_view>& arguments)
{
    if (contains_argument(arguments, "--help")) {
        return {
            .options = std::nullopt,
            .show_help = true,
            .show_version = false,
            .error = {}
        };
    }

    if (contains_argument(arguments, "--version")) {
        return {
            .options = std::nullopt,
            .show_help = false,
            .show_version = true,
            .error = {}
        };
    }

    if (arguments.empty()) {
        return make_error(
            "command is required; use --help for usage");
    }

    CommandLineOptions options;

    if (arguments[0] == "analyze") {
        options.command = Command::Analyze;
    } else if (arguments[0] == "export") {
        options.command = Command::Export;
    } else {
        return make_error(
            "unknown command: " +
            std::string{arguments[0]});
    }

    bool input_received = false;
    bool output_received = false;
    bool level_received = false;
    bool service_received = false;
    bool from_received = false;
    bool to_received = false;
    bool top_errors_received = false;

    std::size_t index = 1;

    while (index < arguments.size()) {
        const std::string_view option =
            arguments[index];

        if (!is_option(option)) {
            return make_error(
                "unexpected argument: " +
                std::string{option});
        }

        if (index + 1 >= arguments.size() ||
            is_option(arguments[index + 1])) {
            return make_error(
                "missing value for option: " +
                std::string{option});
        }

        const std::string_view value =
            arguments[index + 1];

        if (option == "--input") {
            if (input_received) {
                return make_error(
                    "option specified more than once: "
                    "--input");
            }

            options.input_path =
                std::filesystem::path{
                    std::string{value}
                };

            input_received = true;
        } else if (option == "--output") {
            if (output_received) {
                return make_error(
                    "option specified more than once: "
                    "--output");
            }

            options.output_path =
                std::filesystem::path{
                    std::string{value}
                };

            output_received = true;
        } else if (option == "--level") {
            if (level_received) {
                return make_error(
                    "option specified more than once: "
                    "--level");
            }

            const auto level =
                log_level_from_string(value);

            if (!level.has_value()) {
                return make_error(
                    "unknown log level: " +
                    std::string{value});
            }

            options.filters.level = level;
            level_received = true;
        } else if (option == "--service") {
            if (service_received) {
                return make_error(
                    "option specified more than once: "
                    "--service");
            }

            if (value.empty()) {
                return make_error(
                    "service name must not be empty");
            }

            options.filters.service =
                std::string{value};

            service_received = true;
        } else if (option == "--from") {
            if (from_received) {
                return make_error(
                    "option specified more than once: "
                    "--from");
            }

            const auto from_date =
                parse_filter_date(value);

            if (!from_date.has_value()) {
                return make_error(
                    "invalid --from date: " +
                    std::string{value});
            }

            options.filters.from_date = from_date;
            from_received = true;
        } else if (option == "--to") {
            if (to_received) {
                return make_error(
                    "option specified more than once: "
                    "--to");
            }

            const auto to_date =
                parse_filter_date(value);

            if (!to_date.has_value()) {
                return make_error(
                    "invalid --to date: " +
                    std::string{value});
            }

            options.filters.to_date = to_date;
            to_received = true;
        } else if (option == "--top-errors") {
            if (top_errors_received) {
                return make_error(
                    "option specified more than once: "
                    "--top-errors");
            }

            const auto limit =
                parse_positive_number(value);

            if (!limit.has_value()) {
                return make_error(
                    "--top-errors must be "
                    "a positive integer");
            }

            options.top_errors_limit = *limit;
            top_errors_received = true;
        } else {
            return make_error(
                "unknown option: " +
                std::string{option});
        }

        index += 2;
    }

    if (!input_received) {
        return make_error(
            "--input is required");
    }

    if (options.command == Command::Analyze &&
        output_received) {
        return make_error(
            "--output is only valid for "
            "the export command");
    }

    if (options.command == Command::Export &&
        !output_received) {
        return make_error(
            "--output is required for "
            "the export command");
    }

    if (options.filters.from_date.has_value() &&
        options.filters.to_date.has_value() &&
        *options.filters.from_date >
            *options.filters.to_date) {
        return make_error(
            "--from date must not be later "
            "than --to date");
    }

    return {
        .options = std::move(options),
        .show_help = false,
        .show_version = false,
        .error = {}
    };
}

std::string_view help_text() noexcept
{
    return
        "Log Analyzer CLI\n"
        "\n"
        "Usage:\n"
        "  log-analyzer analyze --input <path> [options]\n"
        "  log-analyzer export --input <path> "
        "--output <path> [options]\n"
        "  log-analyzer --help\n"
        "  log-analyzer --version\n"
        "\n"
        "Commands:\n"
        "  analyze              Analyze a structured log file\n"
        "  export               Export filtered entries to CSV\n"
        "\n"
        "Options:\n"
        "  --input <path>       Input log file\n"
        "  --output <path>      Output CSV file for export\n"
        "  --level <level>      DEBUG, INFO, WARN or ERROR\n"
        "  --service <name>     Exact service name\n"
        "  --from <YYYY-MM-DD>  Start date, inclusive\n"
        "  --to <YYYY-MM-DD>    End date, inclusive\n"
        "  --top-errors <N>     Number of frequent errors\n"
        "  --help               Show this help message\n"
        "  --version            Show application version\n";
}

std::string_view version_text() noexcept
{
    return "Log Analyzer CLI 0.6.0\n";
}

} // namespace log_analyzer