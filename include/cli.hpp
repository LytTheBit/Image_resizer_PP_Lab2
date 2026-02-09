// cli.hpp
// Created by Francesco on 08/02/2026.
//
// Command-line interface parsing.
// Parses program arguments into a structured configuration used by main.cpp.
#pragma once

#include <string>
#include <ostream>

#include "resize.hpp"

// Run mode determines the main program flow: either run a single resize or a benchmark.
enum class RunMode {
    Run,        // Run a single resize and write output image
    Bench,      // Run a benchmark and write results to CSV
    Validate,   // Compare two images and print difference metrics
    BenchSet,   // Run a set of benchmarks with different parameters (not implemented)
    Help        // Print usage information
};

// CLI options parsed from command-line arguments.
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

    // Bench mode (also used by BenchSet)
    int warmup = 2;
    int runs = 10;
    std::string csv_path;

    // BenchSet mode parameters (size sweep)
    int base_w = 0;     // starting output width
    int base_h = 0;     // starting output height
    int steps = 0;      // number of sizes to test
    double scale = 1.0; // multiplier per step (e.g., 1.5)
};


void print_usage(std::ostream& os);

CliOptions parse_cli(int argc, char** argv);
