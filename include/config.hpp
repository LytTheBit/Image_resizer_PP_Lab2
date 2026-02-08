// config.hpp
// Created by Francesco on 08/02/2026.
//
// Project-wide default settings (CLI defaults, I/O defaults).
// Centralizes common constants to keep CLI and main.cpp minimal.
#pragma once

namespace cfg {
    inline constexpr int default_threads = 0;        // 0 => OpenMP decides / or sequential ignores
    inline constexpr int default_warmup_runs = 2;
    inline constexpr int default_measured_runs = 10;

    inline constexpr int default_png_compression = 3; // 0..9 (stb)
    inline constexpr int default_jpg_quality = 95;    // 1..100 (stb)

    inline constexpr const char* default_csv_path = "benchmark_results.csv";
}
