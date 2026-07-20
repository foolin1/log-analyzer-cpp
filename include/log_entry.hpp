#pragma once

#include "log_level.hpp"

#include <chrono>
#include <cstdint>
#include <string>

namespace log_analyzer {

struct LogEntry {
    std::chrono::sys_seconds timestamp;
    LogLevel level;
    std::string service;
    std::string message;
    std::int64_t duration_ms;
};

} // namespace log_analyzer