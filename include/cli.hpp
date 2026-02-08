// cli.hpp
// Created by Francesco on 08/02/2026.
//
// Command-line interface parsing.
// Parses program arguments into a structured configuration used by main.cpp.
#pragma once

#include <string>
#include <ostream>

#include "resize.hpp"

enum class RunMode {
    Run,
    Bench,
    Help
};

struct CliOptions {
    RunMode mode = RunMode::Help;

    // Common
    std::string input_path;
    ResizeMethod method = ResizeMethod::Nearest;
    Backend backend = Backend::Sequential;
    int threads = 0;

    // Run mode
    std::string output_path;
    int out_w = 0;
    int out_h = 0;

    // Bench mode
    int warmup = 2;
    int runs = 10;
    std::string csv_path;
};

void print_usage(std::ostream& os);

CliOptions parse_cli(int argc, char** argv);
