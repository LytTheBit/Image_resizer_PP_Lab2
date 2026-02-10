// main.cpp
// Created by Francesco on 07/02/2026.
//
// Program entry point.
// If no CLI arguments are provided, an automatic experimental protocol is executed
// (validation + benchmark sweep) on a fixed input image, for reproducibility.
#include <iostream>
#include <exception>
#include <filesystem>
#include <cmath>

#include "cli.hpp"
#include "io.hpp"
#include "benchmark.hpp"
#include "config.hpp"
#include "util.hpp"
#include "validate.hpp"

int main(int argc, char** argv) {
    try {
        // ================================================================
        // AUTOMATIC TEST MODE (no CLI arguments)
        // ================================================================
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
            const int inner_reps = 10; // repeated resizes per measured run

            // ------------------------------------------------------------
            // 1) Correctness validation (seq vs OpenMP)
            // ------------------------------------------------------------
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

            // ------------------------------------------------------------
            // 2) Benchmark sweep (size scaling)
            // ------------------------------------------------------------
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
                "backend,out_w,out_h,channels,inner_reps,mean_ms,stddev_ms,min_ms,max_ms";

            for (int i = 0; i < steps; ++i) {
                std::cout << "\n[STEP " << (i + 1) << "/" << steps
                          << "] size = " << w << "x" << h << "\n";

                // --- Sequential ---
                {
                    std::cout << "  seq  : running... " << std::flush;

                    BenchResult r = benchmark_resize(
                        img, w, h,
                        method, Backend::Sequential, 0,
                        warmup, runs,
                        inner_reps
                    );

                    std::cout << "mean = " << r.mean_ms << " ms\n";

                    append_csv_row(
                        "bench_seq.csv",
                        header,
                        "seq," + std::to_string(w) + "," + std::to_string(h) + "," +
                        std::to_string(img.channels) + "," +
                        std::to_string(inner_reps) + "," +
                        std::to_string(r.mean_ms) + "," +
                        std::to_string(r.stddev_ms) + "," +
                        std::to_string(r.min_ms) + "," +
                        std::to_string(r.max_ms)
                    );
                }

                // --- OpenMP ---
                {
                    std::cout << "  omp  : running... " << std::flush;

                    BenchResult r = benchmark_resize(
                        img, w, h,
                        method, Backend::OpenMP, threads,
                        warmup, runs,
                        inner_reps
                    );

                    std::cout << "mean = " << r.mean_ms << " ms\n";

                    append_csv_row(
                        "bench_omp.csv",
                        header,
                        "omp," + std::to_string(w) + "," + std::to_string(h) + "," +
                        std::to_string(img.channels) + "," +
                        std::to_string(inner_reps) + "," +
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

        // ================================================================
        // CLI MODE
        // ================================================================
        const CliOptions opt = parse_cli(argc, argv);

        if (opt.mode == RunMode::Help) {
            print_usage(std::cerr);
            return 1;
        }

        // ------------------ VALIDATE ------------------
        if (opt.mode == RunMode::Validate) {
            Image img = load_image(opt.input_path, 0);

            Image out_seq = resize(img, opt.out_w, opt.out_h, opt.method,
                                   Backend::Sequential, 0);
            Image out_omp = resize(img, opt.out_w, opt.out_h, opt.method,
                                   Backend::OpenMP, opt.threads);

            DiffStats d = compare_images(out_seq, out_omp);

            std::cout << "VALIDATE\n"
                      << "  input            = " << opt.input_path << "\n"
                      << "  out_w,out_h       = " << opt.out_w << "," << opt.out_h << "\n"
                      << "  method            = "
                      << ((opt.method == ResizeMethod::Nearest) ? "nearest" : "bilinear") << "\n"
                      << "  omp_threads       = " << opt.threads << "\n"
                      << "  different_values  = " << d.different_values << "\n"
                      << "  max_abs_diff      = " << d.max_abs_diff << "\n";

            return (d.different_values == 0) ? 0 : 3;
        }

        // ------------------ RUN ------------------
        if (opt.mode == RunMode::Run) {
            Image img = load_image(opt.input_path, 0);
            Image out = resize(img, opt.out_w, opt.out_h,
                               opt.method, opt.backend, opt.threads);

            if (ends_with_icase(opt.output_path, ".jpg") ||
                ends_with_icase(opt.output_path, ".jpeg")) {
                save_jpg(out, opt.output_path, cfg::default_jpg_quality);
            } else {
                save_png(out, opt.output_path, cfg::default_png_compression);
            }

            std::cout << "OK: wrote " << opt.output_path
                      << " (" << out.width << "x" << out.height
                      << "x" << out.channels << ")\n";
            return 0;
        }

        // ------------------ BENCH ------------------
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

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 2;
    }
}
