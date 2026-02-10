// benchmark.hpp
// Created by Francesco on 07/02/2026.
//
// Declares benchmarking utilities for image resizing.
// Defines data structures for statistical results and functions
// to run repeated measurements with warmup and export results to CSV.

#pragma once

#include <vector>
#include <string>
#include "image.hpp"
#include "resize.hpp"

// Data structure to hold benchmark results.
struct BenchResult {
    int runs = 0;
    double mean_ms = 0.0;
    double stddev_ms = 0.0;
    double min_ms = 0.0;
    double max_ms = 0.0;
};

// Run a benchmark of the resize function with the given parameters.
BenchResult benchmark_resize(
    const Image& img,
    int out_w, int out_h,
    ResizeMethod method,
    Backend backend,
    int threads,
    int warmup,
    int runs,
    int inner_reps = 1   // default per compatibilità
);


void append_csv_row(
    const std::string& csv_path,
    const std::string& header_if_new,
    const std::string& row
);
