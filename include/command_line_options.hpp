#pragma once

#include "log_filter.hpp"

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace log_analyzer {

enum class Command {
    Analyze,
    Export
};

struct CommandLineOptions {
    Command command{Command::Analyze};
    std::filesystem::path input_path;
    std::optional<std::filesystem::path> output_path;
    LogFilterOptions filters;
    std::size_t top_errors_limit{5};
};

struct CommandLineParseResult {
    std::optional<CommandLineOptions> options;
    bool show_help{};
    bool show_version{};
    std::string error;

    [[nodiscard]] bool success() const noexcept
    {
        return error.empty();
    }
};

class CommandLineOptionsParser {
public:
    [[nodiscard]] static CommandLineParseResult parse(
        const std::vector<std::string_view>& arguments);
};

[[nodiscard]] std::string_view help_text() noexcept;
[[nodiscard]] std::string_view version_text() noexcept;

} // namespace log_analyzer