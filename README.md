# Log Analyzer CLI

[![CI](https://github.com/foolin1/log-analyzer-cpp/actions/workflows/ci.yml/badge.svg)](https://github.com/foolin1/log-analyzer-cpp/actions/workflows/ci.yml)

A command-line application written in modern C++ for parsing, filtering,
analyzing, and exporting structured application logs.

The project demonstrates practical use of C++20, CMake, STL containers,
filesystem operations, error handling, automated tests, and continuous
integration.

## Features

- Parsing structured log files line by line
- Validation of timestamps, log levels, messages, and durations
- Support for `DEBUG`, `INFO`, `WARN`, and `ERROR` levels
- Filtering by:
  - log level;
  - service name;
  - start date;
  - end date
- Combined filtering using logical AND
- Statistics by log level and service
- Minimum, maximum, average, and median response time
- Top frequent `ERROR` messages
- CSV export with correct escaping of commas and quotes
- Graceful handling of malformed log lines
- Automated tests with Catch2
- CI builds on Windows and Ubuntu

## Log format

Each valid log line has the following format:

```text
YYYY-MM-DDTHH:MM:SS LEVEL SERVICE MESSAGE DURATION_MS
```

Example:

```text
2026-07-09T10:15:30 INFO auth User logged in 125
2026-07-09T10:16:04 ERROR payments Payment failed 840
2026-07-09T10:17:11 WARN api Slow request detected 1250
```

Fields:

| Field | Description |
|---|---|
| Timestamp | Date and time in `YYYY-MM-DDTHH:MM:SS` format |
| Level | `DEBUG`, `INFO`, `WARN`, or `ERROR` |
| Service | Service name without spaces |
| Message | Log message containing one or more words |
| Duration | Non-negative duration in milliseconds |

Malformed lines are counted as invalid and skipped. Processing of the
remaining file continues.

## Requirements

- C++20-compatible compiler
- CMake 3.20 or newer
- Git
- Visual Studio 2022 Build Tools on Windows
- GCC or Clang on Linux

Catch2 is downloaded automatically by CMake during configuration.

## Build on Windows

Open PowerShell in the project directory:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

The executable will be created at:

```text
build\Release\log-analyzer.exe
```

## Build on Linux

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

The executable will be created at:

```text
build/log-analyzer
```

## Usage

### Help

Windows:

```powershell
.\build\Release\log-analyzer.exe --help
```

Linux:

```bash
./build/log-analyzer --help
```

### Version

```powershell
.\build\Release\log-analyzer.exe --version
```

Output:

```text
Log Analyzer CLI 1.0.0
```

## Analyze logs

Analyze the entire file:

```powershell
.\build\Release\log-analyzer.exe analyze `
  --input samples\application.log
```

Filter by level:

```powershell
.\build\Release\log-analyzer.exe analyze `
  --input samples\application.log `
  --level ERROR
```

Apply several filters:

```powershell
.\build\Release\log-analyzer.exe analyze `
  --input samples\application.log `
  --level ERROR `
  --service payments `
  --from 2026-07-09 `
  --to 2026-07-10 `
  --top-errors 3
```

Dates passed through `--from` and `--to` are inclusive.

Service filtering is case-sensitive.

## Example report

```text
Log file: samples\application.log
Parsed entries: 15
Invalid lines: 5
Filtered entries: 15
Levels:
 DEBUG: 2
 INFO: 5
 WARN: 2
 ERROR: 6
Services:
 api: 4
 auth: 3
 database: 3
 payments: 5
Response time, ms:
 Minimum: 12
 Maximum: 3000
 Average: 869.7
 Median: 430.0
Top error messages:
 1. Payment failed - 3
 2. Database timeout - 2
 3. External API unavailable - 1
```

## Export to CSV

Export all error entries:

```powershell
.\build\Release\log-analyzer.exe export `
  --input samples\application.log `
  --level ERROR `
  --output errors.csv
```

Export filtered entries:

```powershell
.\build\Release\log-analyzer.exe export `
  --input samples\application.log `
  --level ERROR `
  --service payments `
  --from 2026-07-09 `
  --to 2026-07-09 `
  --output payment-errors.csv
```

CSV structure:

```text
timestamp,level,service,message,duration_ms
2026-07-09T10:16:04,ERROR,payments,Payment failed,840
```

Fields containing commas, quotation marks, or line breaks are enclosed in
quotation marks. Embedded quotation marks are doubled according to CSV
escaping rules.

## Command-line options

| Option | Description |
|---|---|
| `analyze` | Analyze a log file and print statistics |
| `export` | Export filtered records to CSV |
| `--input <path>` | Path to the input log file |
| `--output <path>` | Path to the output CSV file |
| `--level <level>` | Filter by log level |
| `--service <name>` | Filter by exact service name |
| `--from <date>` | Inclusive start date |
| `--to <date>` | Inclusive end date |
| `--top-errors <N>` | Number of frequent error messages |
| `--help` | Display help |
| `--version` | Display application version |

## Exit codes

| Code | Meaning |
|---:|---|
| `0` | Successful execution |
| `1` | Invalid command-line arguments |
| `2` | Input file access or reading error |
| `3` | CSV output error |

## Running tests

Windows:

```powershell
ctest --test-dir build -C Release --output-on-failure
```

Linux:

```bash
ctest --test-dir build --output-on-failure
```

The test suite covers:

- valid and invalid log parsing;
- timestamps and durations;
- file reading;
- invalid line counting;
- filtering;
- date boundaries;
- combined filters;
- level and service statistics;
- average and median calculation;
- frequent error messages;
- command-line argument validation;
- CSV generation and escaping;
- file access errors.

## Project structure

```text
log-analyzer-cpp/
├── .github/
│   └── workflows/
│       └── ci.yml
├── app/
│   └── main.cpp
├── include/
│   ├── command_line_options.hpp
│   ├── csv_exporter.hpp
│   ├── log_entry.hpp
│   ├── log_filter.hpp
│   ├── log_level.hpp
│   ├── log_parser.hpp
│   ├── log_reader.hpp
│   └── log_statistics.hpp
├── samples/
│   └── application.log
├── src/
│   ├── command_line_options.cpp
│   ├── csv_exporter.cpp
│   ├── log_filter.cpp
│   ├── log_level.cpp
│   ├── log_parser.cpp
│   ├── log_reader.cpp
│   └── log_statistics.cpp
├── tests/
│   ├── command_line_options_tests.cpp
│   ├── csv_exporter_tests.cpp
│   ├── log_filter_tests.cpp
│   ├── log_parser_tests.cpp
│   ├── log_reader_tests.cpp
│   └── log_statistics_tests.cpp
├── .gitignore
├── CMakeLists.txt
└── README.md
```

## Architecture

The application is divided into independent modules:

- `LogParser` converts a text line into a validated `LogEntry`;
- `LogReader` reads the input file and counts malformed lines;
- `LogFilter` applies optional filtering conditions;
- `LogStatistics` calculates aggregate metrics;
- `CsvExporter` writes filtered entries to CSV;
- `CommandLineOptionsParser` validates command-line arguments;
- `main.cpp` coordinates application modules and exit codes.

The core application logic is placed in the `log_analyzer_core` library,
which is shared by the CLI executable and automated tests.

## Continuous integration

GitHub Actions automatically builds and tests the project on:

- Windows with Visual Studio 2022;
- Ubuntu with the default C++ compiler.

The workflow runs for pushes and pull requests targeting the `main` branch.

## Technology stack

- C++20
- CMake
- Catch2
- STL
- Git
- GitHub Actions

## Author

GitHub: [foolin1](https://github.com/foolin1)