cmake_minimum_required(VERSION 3.20)

project(
    LogAnalyzerCLI
    VERSION 0.6.0
    DESCRIPTION
        "Command-line application for analyzing structured log files"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

function(enable_project_warnings target_name)
    if(MSVC)
        target_compile_options(
            ${target_name}
            PRIVATE
                /W4
                /permissive-
        )
    else()
        target_compile_options(
            ${target_name}
            PRIVATE
                -Wall
                -Wextra
                -Wpedantic
        )
    endif()
endfunction()

add_library(
    log_analyzer_core
    src/log_level.cpp
    src/log_parser.cpp
    src/log_reader.cpp
    src/log_filter.cpp
    src/log_statistics.cpp
    src/command_line_options.cpp
)

target_include_directories(
    log_analyzer_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

enable_project_warnings(log_analyzer_core)

add_executable(
    log-analyzer
    app/main.cpp
)

target_link_libraries(
    log-analyzer
    PRIVATE
        log_analyzer_core
)

enable_project_warnings(log-analyzer)

set_property(
    DIRECTORY
    PROPERTY VS_STARTUP_PROJECT log-analyzer
)

if(BUILD_TESTING)
    include(FetchContent)

    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY
            https://github.com/catchorg/Catch2.git
        GIT_TAG
            v3.15.2
        GIT_SHALLOW
            TRUE
    )

    FetchContent_MakeAvailable(Catch2)

    add_executable(
        log-analyzer-tests
        tests/log_parser_tests.cpp
        tests/log_reader_tests.cpp
        tests/log_filter_tests.cpp
        tests/log_statistics_tests.cpp
        tests/command_line_options_tests.cpp
    )

    target_link_libraries(
        log-analyzer-tests
        PRIVATE
            log_analyzer_core
            Catch2::Catch2WithMain
    )

    enable_project_warnings(log-analyzer-tests)

    list(
        APPEND CMAKE_MODULE_PATH
        ${catch2_SOURCE_DIR}/extras
    )

    include(Catch)

    catch_discover_tests(log-analyzer-tests)
endif()