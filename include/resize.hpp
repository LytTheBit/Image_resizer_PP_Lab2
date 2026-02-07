// resize.hpp
// Created by Francesco on 07/02/2026.
//
// Declares the public interface for image resizing.
// It defines resize methods, execution backends (sequential and OpenMP),
// and exposes a unified resize function used by benchmarks and CLI.

#pragma once

#include "image.hpp"

enum class ResizeMethod {
    Nearest,
    Bilinear
};

enum class Backend {
    Sequential,
    OpenMP
};

Image resize_seq(const Image& in, int out_w, int out_h, ResizeMethod method);
Image resize_omp(const Image& in, int out_w, int out_h, ResizeMethod method, int threads);

// comoda “facciata”
inline Image resize(const Image& in, int out_w, int out_h, ResizeMethod method, Backend backend, int threads) {
    if (backend == Backend::OpenMP) {
        return resize_omp(in, out_w, out_h, method, threads);
    }
    return resize_seq(in, out_w, out_h, method);
}
