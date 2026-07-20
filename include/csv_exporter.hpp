#pragma once

#include "log_entry.hpp"

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace log_analyzer {

struct CsvExportResult {
    std::size_t rows_written{};
    std::string error;

    [[nodiscard]] bool success() const noexcept
    {
        return error.empty();
    }
};

class CsvExporter {
public:
    [[nodiscard]] static CsvExportResult export_entries(
        const std::vector<LogEntry>& entries,
        const std::filesystem::path& output_path);
};

} // namespace log_analyzer