// main.cpp
// Created by Francesco on 07/02/2026.
//
// Program entry point.
// Provides a minimal CLI to run a single resize or a benchmark run.
// The goal is to validate correctness (seq vs OpenMP) and measure performance
// under controlled parameters (warmup + repeated measurements + CSV output).
#include <iostream>
#include <string>
#include <stdexcept>

#include "io.hpp"
#include "resize.hpp"
#include "benchmark.hpp"

static void print_usage() {
    std::cerr
        << "Usage:\n"
        << "  Image_resizer_PP_Lab2 run <input> <output_png> <out_w> <out_h> <nearest|bilinear> <seq|omp> [threads]\n"
        << "  Image_resizer_PP_Lab2 bench <input> <out_w> <out_h> <nearest|bilinear> <seq|omp> [threads] [warmup] [runs] [csv_path]\n"
        << "\nExamples:\n"
        << "  Image_resizer_PP_Lab2 run lena.png out.png 1920 1080 bilinear omp 12\n"
        << "  Image_resizer_PP_Lab2 bench lena.png 3840 2160 bilinear omp 12 2 10 results.csv\n";
}

static ResizeMethod parse_method(const std::string& s) {
    if (s == "nearest") return ResizeMethod::Nearest;
    if (s == "bilinear") return ResizeMethod::Bilinear;
    throw std::invalid_argument("Unknown method: " + s);
}

static Backend parse_backend(const std::string& s) {
    if (s == "seq") return Backend::Sequential;
    if (s == "omp") return Backend::OpenMP;
    throw std::invalid_argument("Unknown backend: " + s);
}

static int parse_int(const std::string& s, const std::string& name) {
    try {
        size_t idx = 0;
        int v = std::stoi(s, &idx);
        if (idx != s.size()) throw std::invalid_argument("");
        return v;
    } catch (...) {
        throw std::invalid_argument("Invalid integer for " + name + ": " + s);
    }
}

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            print_usage();
            return 1;
        }

        const std::string mode = argv[1];

        if (mode == "run") {
            if (argc < 8) {
                print_usage();
                return 1;
            }

            const std::string input_path = argv[2];
            const std::string output_path = argv[3];
            const int out_w = parse_int(argv[4], "out_w");
            const int out_h = parse_int(argv[5], "out_h");
            const ResizeMethod method = parse_method(argv[6]);
            const Backend backend = parse_backend(argv[7]);
            const int threads = (argc >= 9) ? parse_int(argv[8], "threads") : 0;

            Image img = load_image(input_path, 0);
            Image out = resize(img, out_w, out_h, method, backend, threads);

            // Save as PNG (safe for 1/3/4 channels)
            save_png(out, output_path);

            std::cout << "OK: wrote " << output_path
                      << " (" << out.width << "x" << out.height << "x" << out.channels << ")\n";
            return 0;
        }

        if (mode == "bench") {
            if (argc < 7) {
                print_usage();
                return 1;
            }

            const std::string input_path = argv[2];
            const int out_w = parse_int(argv[3], "out_w");
            const int out_h = parse_int(argv[4], "out_h");
            const ResizeMethod method = parse_method(argv[5]);
            const Backend backend = parse_backend(argv[6]);

            const int threads = (argc >= 8) ? parse_int(argv[7], "threads") : 0;
            const int warmup  = (argc >= 9) ? parse_int(argv[8], "warmup")  : 2;
            const int runs    = (argc >= 10) ? parse_int(argv[9], "runs")   : 10;
            const std::string csv_path = (argc >= 11) ? argv[10] : "benchmark_results.csv";

            Image img = load_image(input_path, 0);

            BenchResult r = benchmark_resize(img, out_w, out_h, method, backend, threads, warmup, runs);

            std::cout << "Benchmark results:\n"
                      << "  runs   = " << r.runs << "\n"
                      << "  mean   = " << r.mean_ms << " ms\n"
                      << "  stddev = " << r.stddev_ms << " ms\n"
                      << "  min    = " << r.min_ms << " ms\n"
                      << "  max    = " << r.max_ms << " ms\n";

            const std::string header =
                "input,out_w,out_h,channels,method,backend,threads,warmup,runs,mean_ms,stddev_ms,min_ms,max_ms";

            auto method_s  = (method == ResizeMethod::Nearest) ? "nearest" : "bilinear";
            auto backend_s = (backend == Backend::Sequential) ? "seq" : "omp";

            const std::string row =
                input_path + "," +
                std::to_string(out_w) + "," +
                std::to_string(out_h) + "," +
                std::to_string(img.channels) + "," +
                method_s + "," +
                backend_s + "," +
                std::to_string(threads) + "," +
                std::to_string(warmup) + "," +
                std::to_string(runs) + "," +
                std::to_string(r.mean_ms) + "," +
                std::to_string(r.stddev_ms) + "," +
                std::to_string(r.min_ms) + "," +
                std::to_string(r.max_ms);

            append_csv_row(csv_path, header, row);
            std::cout << "CSV appended: " << csv_path << "\n";
            return 0;
        }

        print_usage();
        return 1;

    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return 2;
    }
}
