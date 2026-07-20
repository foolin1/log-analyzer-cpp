#pragma once

#include "log_entry.hpp"

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace log_analyzer {

struct LogReadResult {
    std::vector<LogEntry> entries;
    std::size_t total_lines{};
    std::size_t invalid_lines{};
    std::string error;

    [[nodiscard]] bool success() const noexcept
    {
        return error.empty();
    }
};

class LogReader {
public:
    [[nodiscard]] static LogReadResult read(
        const std::filesystem::path& input_path);
};

} // namespace log_analyzer