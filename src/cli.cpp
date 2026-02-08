// cli.cpp
// Created by Francesco on 08/02/2026.
//
// CLI parsing implementation.
// Supports: run, bench. Produces helpful usage text on invalid input.
#include "cli.hpp"

#include "config.hpp"
#include "util.hpp"

#include <stdexcept>

static ResizeMethod parse_method(std::string s) {
    s = to_lower(std::move(s));
    if (s == "nearest")  return ResizeMethod::Nearest;
    if (s == "bilinear") return ResizeMethod::Bilinear;
    throw std::invalid_argument("Unknown method: " + s);
}

static Backend parse_backend(std::string s) {
    s = to_lower(std::move(s));
    if (s == "seq") return Backend::Sequential;
    if (s == "omp") return Backend::OpenMP;
    throw std::invalid_argument("Unknown backend: " + s);
}

void print_usage(std::ostream& os) {
    os
        << "Usage:\n"
        << "  Image_resizer_PP_Lab2 run <input> <output_png|output_jpg> <out_w> <out_h> <nearest|bilinear> <seq|omp> [threads]\n"
        << "  Image_resizer_PP_Lab2 bench <input> <out_w> <out_h> <nearest|bilinear> <seq|omp> [threads] [warmup] [runs] [csv_path]\n"
        << "\nExamples:\n"
        << "  Image_resizer_PP_Lab2 run lena.png out.png 1920 1080 bilinear omp 12\n"
        << "  Image_resizer_PP_Lab2 bench lena.png 3840 2160 bilinear omp 12 2 10 results.csv\n";
}

CliOptions parse_cli(int argc, char** argv) {
    CliOptions opt;
    opt.csv_path = cfg::default_csv_path;
    opt.threads = cfg::default_threads;
    opt.warmup  = cfg::default_warmup_runs;
    opt.runs    = cfg::default_measured_runs;

    if (argc < 2) {
        opt.mode = RunMode::Help;
        return opt;
    }

    const std::string mode = to_lower(argv[1]);

    if (mode == "run") {
        if (argc < 8) {
            opt.mode = RunMode::Help;
            return opt;
        }
        opt.mode = RunMode::Run;
        opt.input_path  = argv[2];
        opt.output_path = argv[3];
        opt.out_w = parse_int(argv[4], "out_w");
        opt.out_h = parse_int(argv[5], "out_h");
        opt.method  = parse_method(argv[6]);
        opt.backend = parse_backend(argv[7]);
        if (argc >= 9) opt.threads = parse_int(argv[8], "threads");
        return opt;
    }

    if (mode == "bench") {
        if (argc < 7) {
            opt.mode = RunMode::Help;
            return opt;
        }
        opt.mode = RunMode::Bench;
        opt.input_path = argv[2];
        opt.out_w = parse_int(argv[3], "out_w");
        opt.out_h = parse_int(argv[4], "out_h");
        opt.method  = parse_method(argv[5]);
        opt.backend = parse_backend(argv[6]);

        if (argc >= 8)  opt.threads = parse_int(argv[7], "threads");
        if (argc >= 9)  opt.warmup  = parse_int(argv[8], "warmup");
        if (argc >= 10) opt.runs    = parse_int(argv[9], "runs");
        if (argc >= 11) opt.csv_path = argv[10];

        return opt;
    }

    opt.mode = RunMode::Help;
    return opt;
}
