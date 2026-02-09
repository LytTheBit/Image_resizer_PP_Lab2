// main.cpp
// Created by Francesco on 07/02/2026.
//
// Program entry point.
// Uses CLI parsing to run a single resize (run) or a benchmark (bench).
// This file is intentionally small: I/O, benchmarking and resize logic are in dedicated modules.
#include <iostream>
#include <exception>
#include <filesystem>

#include "cli.hpp"
#include "io.hpp"
#include "benchmark.hpp"
#include "config.hpp"
#include "util.hpp"

int main(int argc, char** argv) {
    try {
        // Convenience default: if no CLI args are provided, run a local smoke test.
        // This avoids dealing with IDE run configurations.
        if (argc == 1) {
            const std::string input  = "test_1.png";  // expected in working directory
            const std::string output = "out.png";

            if (!std::filesystem::exists(input)) {
                std::cerr
                    << "ERROR: no arguments provided and default input file not found.\n"
                    << "Searched for: " << std::filesystem::absolute(input).string() << "\n\n";
                print_usage(std::cerr);
                return 2;
            }

            // Default parameters for a quick test
            const int out_w = 896;
            const int out_h = 896;
            const ResizeMethod method = ResizeMethod::Bilinear;
            const Backend backend = Backend::OpenMP;
            const int threads = 12; // set 0 to let OpenMP decide

            Image img = load_image(input, 0);
            Image out = resize(img, out_w, out_h, method, backend, threads);

            // Choose output writer based on extension
            if (ends_with_icase(output, ".jpg") || ends_with_icase(output, ".jpeg")) {
                save_jpg(out, output, cfg::default_jpg_quality);
            } else {
                save_png(out, output, cfg::default_png_compression);
            }

            std::cout << "OK (default run): " << input << " -> " << output
                      << " (" << out.width << "x" << out.height << "x" << out.channels << ")\n";
            return 0;
        }

        const CliOptions opt = parse_cli(argc, argv);

        if (opt.mode == RunMode::Help) {
            print_usage(std::cerr);
            return 1;
        }

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
