// benchmark.cpp
// Created by Francesco on 07/02/2026.
//
// Implements benchmarking logic for image resizing.
// Includes warmup handling, timing collection, statistical analysis
// (mean, standard deviation, min/max), and CSV result logging.

#include "benchmark.hpp"
#include "timing.hpp"

#include <fstream>
#include <cmath>
#include <algorithm>
#include <stdexcept>

static double mean(const std::vector<double>& v) {
    double s = 0.0;
    for (double x : v) s += x;
    return s / static_cast<double>(v.size());
}

static double stddev_sample(const std::vector<double>& v, double mu) {
    if (v.size() < 2) return 0.0;
    double s2 = 0.0;
    for (double x : v) {
        const double d = x - mu;
        s2 += d * d;
    }
    return std::sqrt(s2 / static_cast<double>(v.size() - 1));
}

BenchResult benchmark_resize(
    const Image& input,
    int out_w, int out_h,
    ResizeMethod method,
    Backend backend,
    int threads,
    int warmup_runs,
    int measured_runs
) {
    if (input.empty()) throw std::invalid_argument("benchmark_resize: input image is empty");
    if (out_w <= 0 || out_h <= 0) throw std::invalid_argument("benchmark_resize: invalid output size");
    if (warmup_runs < 0 || measured_runs <= 0) throw std::invalid_argument("benchmark_resize: invalid runs");

    // Warmup (cache, JIT-like effects, alloc patterns)
    for (int i = 0; i < warmup_runs; ++i) {
        volatile auto tmp = resize(input, out_w, out_h, method, backend, threads);
        (void)tmp;
    }

    std::vector<double> times;
    times.reserve(static_cast<size_t>(measured_runs));

    for (int i = 0; i < measured_runs; ++i) {
        double t = time_ms([&]() {
            volatile auto out = resize(input, out_w, out_h, method, backend, threads);
            (void)out;
        });
        times.push_back(t);
    }

    std::sort(times.begin(), times.end());

    BenchResult r;
    r.runs = measured_runs;
    r.min_ms = times.front();
    r.max_ms = times.back();
    r.mean_ms = mean(times);
    r.stddev_ms = stddev_sample(times, r.mean_ms);
    return r;
}

void append_csv_row(const std::string& csv_path,
                    const std::string& header_if_new,
                    const std::string& row) {
    // If file doesn't exist or is empty, write header first.
    bool write_header = false;
    {
        std::ifstream in(csv_path, std::ios::binary);
        if (!in.good()) {
            write_header = true;
        } else {
            in.seekg(0, std::ios::end);
            write_header = (in.tellg() == 0);
        }
    }

    std::ofstream out(csv_path, std::ios::app);
    if (!out) throw std::runtime_error("append_csv_row: cannot open file: " + csv_path);

    if (write_header && !header_if_new.empty()) {
        out << header_if_new << "\n";
    }
    out << row << "\n";
}
