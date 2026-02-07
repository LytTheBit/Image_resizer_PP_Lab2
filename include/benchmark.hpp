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

struct BenchResult {
    int runs = 0;
    double mean_ms = 0.0;
    double stddev_ms = 0.0;
    double min_ms = 0.0;
    double max_ms = 0.0;
};

BenchResult benchmark_resize(
    const Image& input,
    int out_w, int out_h,
    ResizeMethod method,
    Backend backend,
    int threads,
    int warmup_runs,
    int measured_runs
);

void append_csv_row(
    const std::string& csv_path,
    const std::string& header_if_new,
    const std::string& row
);
