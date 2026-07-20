cmake_minimum_required(VERSION 3.20)

project(
    LogAnalyzerCLI
    VERSION 0.1.0
    DESCRIPTION "Command-line application for analyzing structured log files"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(
    log-analyzer
    app/main.cpp
)

target_include_directories(
    log-analyzer
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(
        log-analyzer
        PRIVATE
            /W4
            /permissive-
    )
else()
    target_compile_options(
        log-analyzer
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )
endif()

set_property(
    DIRECTORY
    PROPERTY VS_STARTUP_PROJECT log-analyzer
)