// main.cpp
// Created by Francesco on 07/02/2026.
//
// Program entry point.
// Uses CLI parsing to run a single resize (run) or a benchmark (bench).
// This file is intentionally small: I/O, benchmarking and resize logic are in dedicated modules.
#include <iostream>
#include <exception>
#include <filesystem>
#include <cmath>
#include <algorithm>

#include "cli.hpp"
#include "io.hpp"
#include "benchmark.hpp"
#include "config.hpp"
#include "util.hpp"
#include "validate.hpp"

int main(int argc, char** argv) {
    try {
        // Convenience default: if no CLI args are provided, run a local smoke test.
        // This avoids dealing with IDE run configurations.
        if (argc == 1) {
            const std::string input = "test_1.png";

            if (!std::filesystem::exists(input)) {
                std::cerr
                    << "ERROR: default test image not found.\n"
                    << "Expected: " << std::filesystem::absolute(input).string() << "\n";
                return 2;
            }

            // Fixed experimental setup (reproducible)
            const ResizeMethod method = ResizeMethod::Bilinear;
            const int threads = 12;

            // ---------- 1) Correctness test ----------
            {
                std::cout << "\n=== VALIDATION TEST ===\n";

                const int out_w = 896;
                const int out_h = 896;

                Image img = load_image(input, 0);
                Image out_seq = resize(img, out_w, out_h, method, Backend::Sequential, 0);
                Image out_omp = resize(img, out_w, out_h, method, Backend::OpenMP, threads);

                DiffStats d = compare_images(out_seq, out_omp);

                std::cout
                    << "different_values = " << d.different_values << "\n"
                    << "max_abs_diff     = " << d.max_abs_diff << "\n";

                if (d.different_values != 0) {
                    std::cerr << "VALIDATION FAILED\n";
                    return 3;
                }

                std::cout << "VALIDATION PASSED\n";
            }

            // ---------- 2) Benchmark sweep ----------
            std::cout << "\n=== BENCHMARK SWEEP ===\n";

            const int base_w = 512;
            const int base_h = 512;
            const int steps  = 6;
            const double scale = 1.5;
            const int warmup = 2;
            const int runs   = 20;

            Image img = load_image(input, 0);

            int w = base_w;
            int h = base_h;

            const std::string header =
                "backend,out_w,out_h,channels,mean_ms,stddev_ms,min_ms,max_ms";

            for (int i = 0; i < steps; ++i) {
                // Sequential
                {
                    BenchResult r = benchmark_resize(
                        img, w, h,
                        method, Backend::Sequential, 0,
                        warmup, runs
                    );

                    append_csv_row(
                        "bench_seq.csv",
                        header,
                        "seq," + std::to_string(w) + "," + std::to_string(h) + "," +
                        std::to_string(img.channels) + "," +
                        std::to_string(r.mean_ms) + "," +
                        std::to_string(r.stddev_ms) + "," +
                        std::to_string(r.min_ms) + "," +
                        std::to_string(r.max_ms)
                    );
                }

                // OpenMP
                {
                    BenchResult r = benchmark_resize(
                        img, w, h,
                        method, Backend::OpenMP, threads,
                        warmup, runs
                    );

                    append_csv_row(
                        "bench_omp.csv",
                        header,
                        "omp," + std::to_string(w) + "," + std::to_string(h) + "," +
                        std::to_string(img.channels) + "," +
                        std::to_string(r.mean_ms) + "," +
                        std::to_string(r.stddev_ms) + "," +
                        std::to_string(r.min_ms) + "," +
                        std::to_string(r.max_ms)
                    );
                }

                w = static_cast<int>(std::round(w * scale));
                h = static_cast<int>(std::round(h * scale));
            }

            std::cout << "\nEXPERIMENT COMPLETED\n";
            std::cout << "CSV files generated: bench_seq.csv, bench_omp.csv\n";

            return 0;
        }

        const CliOptions opt = parse_cli(argc, argv);

        if (opt.mode == RunMode::Help) {
            print_usage(std::cerr);
            return 1;
        }

        // Validate: compare sequential vs OpenMP outputs (correctness check)
        if (opt.mode == RunMode::Validate) {
            Image img = load_image(opt.input_path, 0);

            Image out_seq = resize(img, opt.out_w, opt.out_h, opt.method, Backend::Sequential, 0);
            Image out_omp = resize(img, opt.out_w, opt.out_h, opt.method, Backend::OpenMP, opt.threads);

            DiffStats d = compare_images(out_seq, out_omp);

            std::cout << "VALIDATE\n"
                      << "  input            = " << opt.input_path << "\n"
                      << "  out_w,out_h       = " << opt.out_w << "," << opt.out_h << "\n"
                      << "  method            = " << ((opt.method == ResizeMethod::Nearest) ? "nearest" : "bilinear") << "\n"
                      << "  omp_threads       = " << opt.threads << "\n"
                      << "  different_values  = " << d.different_values << "\n"
                      << "  max_abs_diff      = " << d.max_abs_diff << "\n";

            if (d.different_values == 0) {
                std::cout << "OK: outputs match.\n";
                return 0;
            } else {
                std::cerr << "FAIL: outputs differ.\n";
                return 3;
            }
        }

        // BenchSet: sweep output sizes and append CSV for each point
        if (opt.mode == RunMode::BenchSet) {
            Image img = load_image(opt.input_path, 0);

            const std::string header =
                "input,out_w,out_h,channels,method,backend,threads,warmup,runs,mean_ms,stddev_ms,min_ms,max_ms";

            const std::string method_s  = (opt.method == ResizeMethod::Nearest) ? "nearest" : "bilinear";
            const std::string backend_s = (opt.backend == Backend::Sequential) ? "seq" : "omp";

            int w = opt.base_w;
            int h = opt.base_h;

            for (int i = 0; i < opt.steps; ++i) {
                // guard against invalid/overflow sizes
                w = clamp_int(w, 1, 100000);
                h = clamp_int(h, 1, 100000);

                BenchResult r = benchmark_resize(
                    img,
                    w, h,
                    opt.method,
                    opt.backend,
                    opt.threads,
                    opt.warmup,
                    opt.runs
                );

                const std::string row =
                    opt.input_path + "," +
                    std::to_string(w) + "," +
                    std::to_string(h) + "," +
                    std::to_string(img.channels) + "," +
                    method_s + "," +
                    backend_s + "," +
                    std::to_string(opt.threads) + "," +
                    std::to_string(opt.warmup) + "," +
                    std::to_string(opt.runs) + "," +
                    std::to_string(r.mean_ms) + "," +
                    std::to_string(r.stddev_ms) + "," +
                    std::to_string(r.min_ms) + "," +
                    std::to_string(r.max_ms);

                append_csv_row(opt.csv_path, header, row);

                std::cout << "BENCHSET [" << (i + 1) << "/" << opt.steps << "]: "
                          << w << "x" << h
                          << " mean=" << r.mean_ms << " ms"
                          << " (backend=" << backend_s << ", method=" << method_s
                          << ", threads=" << opt.threads << ")\n";

                // next size
                const double next_w = std::round(static_cast<double>(w) * opt.scale);
                const double next_h = std::round(static_cast<double>(h) * opt.scale);
                w = static_cast<int>(next_w);
                h = static_cast<int>(next_h);
            }

            std::cout << "CSV appended: " << opt.csv_path << "\n";
            return 0;
        }

        // Run: single resize and save output
        if (opt.mode == RunMode::Run) {
            Image img = load_image(opt.input_path, 0);
            Image out = resize(img, opt.out_w, opt.out_h, opt.method, opt.backend, opt.threads);

            // Choose output writer based on extension
            if (ends_with_icase(opt.output_path, ".jpg") || ends_with_icase(opt.output_path, ".jpeg")) {
                save_jpg(out, opt.output_path, cfg::default_jpg_quality);
            } else {
                save_png(out, opt.output_path, cfg::default_png_compression);
            }

            std::cout << "OK: wrote " << opt.output_path
                      << " (" << out.width << "x" << out.height << "x" << out.channels << ")\n";
            return 0;
        }

        // Bench: single-point benchmark and CSV append
        // Bench
        Image img = load_image(opt.input_path, 0);

        BenchResult r = benchmark_resize(
            img,
            opt.out_w, opt.out_h,
            opt.method,
            opt.backend,
            opt.threads,
            opt.warmup,
            opt.runs
        );

        std::cout << "Benchmark results:\n"
                  << "  runs   = " << r.runs << "\n"
                  << "  mean   = " << r.mean_ms << " ms\n"
                  << "  stddev = " << r.stddev_ms << " ms\n"
                  << "  min    = " << r.min_ms << " ms\n"
                  << "  max    = " << r.max_ms << " ms\n";

        const std::string header =
            "input,out_w,out_h,channels,method,backend,threads,warmup,runs,mean_ms,stddev_ms,min_ms,max_ms";

        const std::string method_s  = (opt.method == ResizeMethod::Nearest) ? "nearest" : "bilinear";
        const std::string backend_s = (opt.backend == Backend::Sequential) ? "seq" : "omp";

        const std::string row =
            opt.input_path + "," +
            std::to_string(opt.out_w) + "," +
            std::to_string(opt.out_h) + "," +
            std::to_string(img.channels) + "," +
            method_s + "," +
            backend_s + "," +
            std::to_string(opt.threads) + "," +
            std::to_string(opt.warmup) + "," +
            std::to_string(opt.runs) + "," +
            std::to_string(r.mean_ms) + "," +
            std::to_string(r.stddev_ms) + "," +
            std::to_string(r.min_ms) + "," +
            std::to_string(r.max_ms);

        append_csv_row(opt.csv_path, header, row);
        std::cout << "CSV appended: " << opt.csv_path << "\n";

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 2;
    }
}