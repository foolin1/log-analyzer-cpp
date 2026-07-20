#include "log_reader.hpp"

#include "log_parser.hpp"

#include <fstream>
#include <string>
#include <system_error>
#include <utility>

namespace log_analyzer {

namespace {

[[nodiscard]] std::string path_to_string(
    const std::filesystem::path& path)
{
    return path.string();
}

} // namespace

LogReadResult LogReader::read(
    const std::filesystem::path& input_path)
{
    LogReadResult result;

    std::error_code filesystem_error;

    const bool file_exists =
        std::filesystem::exists(
            input_path,
            filesystem_error);

    if (filesystem_error) {
        result.error =
            "cannot access input path: " +
            path_to_string(input_path);

        return result;
    }

    if (!file_exists) {
        result.error =
            "input file does not exist: " +
            path_to_string(input_path);

        return result;
    }

    const bool is_regular_file =
        std::filesystem::is_regular_file(
            input_path,
            filesystem_error);

    if (filesystem_error || !is_regular_file) {
        result.error =
            "input path is not a regular file: " +
            path_to_string(input_path);

        return result;
    }

    std::ifstream input_file{input_path};

    if (!input_file.is_open()) {
        result.error =
            "cannot open input file: " +
            path_to_string(input_path);

        return result;
    }

    std::string line;

    while (std::getline(input_file, line)) {
        ++result.total_lines;

        auto parse_result =
            LogParser::parse(line);

        if (parse_result.success()) {
            result.entries.push_back(
                std::move(*parse_result.entry));
        } else {
            ++result.invalid_lines;
        }
    }

    if (input_file.bad()) {
        result.error =
            "failed to read input file: " +
            path_to_string(input_path);
    }

    return result;
}

} // namespace log_analyzer