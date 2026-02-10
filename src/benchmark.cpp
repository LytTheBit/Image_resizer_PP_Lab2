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
    const Image& img,
    int out_w, int out_h,
    ResizeMethod method,
    Backend backend,
    int threads,
    int warmup,
    int runs,
    int inner_reps
) {
    if (inner_reps <= 0) inner_reps = 1;

    std::vector<double> samples;
    samples.reserve(runs);

    // Warmup
    for (int i = 0; i < warmup; ++i) {
        for (int k = 0; k < inner_reps; ++k) {
            Image out = resize(img, out_w, out_h, method, backend, threads);
        }
    }

    // Measured runs
    for (int i = 0; i < runs; ++i) {
        const double t0 = now_ms();

        for (int k = 0; k < inner_reps; ++k) {
            Image out = resize(img, out_w, out_h, method, backend, threads);
        }

        const double t1 = now_ms();
        const double elapsed = (t1 - t0) / inner_reps;  // normalize
        samples.push_back(elapsed);
    }

    BenchResult r{};
    r.runs = runs;
    r.mean_ms   = mean(samples);
    r.stddev_ms = stddev_sample(samples, r.mean_ms);
    r.min_ms    = *std::min_element(samples.begin(), samples.end());
    r.max_ms    = *std::max_element(samples.begin(), samples.end());

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
