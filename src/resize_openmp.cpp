// resize_openmp.cpp
// Created by Francesco on 07/02/2026.
//
// OpenMP-parallel implementations of image resizing algorithms.
// This file mirrors the sequential resizing logic while exploiting
// data parallelism over image rows using OpenMP, enabling performance
// comparisons between sequential and parallel executions.
#include "resize.hpp"

#include <cmath>
#include <stdexcept>

#if HAVE_OPENMP
  #include <omp.h>
#endif

static inline float map_coord(float out_coord, float in_size, float out_size) {
    return (out_coord + 0.5f) * (in_size / out_size) - 0.5f;
}

static Image resize_nearest_omp(const Image& in, int out_w, int out_h, int threads) {
    Image out(out_w, out_h, in.channels);

#if HAVE_OPENMP
    if (threads > 0) omp_set_num_threads(threads);

    #pragma omp parallel for schedule(static)
    for (int y = 0; y < out_h; ++y) {
        const float sy = map_coord(static_cast<float>(y), static_cast<float>(in.height), static_cast<float>(out_h));
        int iy = static_cast<int>(std::lround(sy));
        iy = clamp_int(iy, 0, in.height - 1);

        for (int x = 0; x < out_w; ++x) {
            const float sx = map_coord(static_cast<float>(x), static_cast<float>(in.width), static_cast<float>(out_w));
            int ix = static_cast<int>(std::lround(sx));
            ix = clamp_int(ix, 0, in.width - 1);

            const std::uint8_t* src = in.row_ptr(iy) + (ix * in.channels);
            std::uint8_t* dst = out.row_ptr(y) + (x * out.channels);

            for (int c = 0; c < out.channels; ++c) dst[c] = src[c];
        }
    }
#else
    (void)threads;
    out = resize_seq(in, out_w, out_h, ResizeMethod::Nearest);
#endif

    return out;
}

static Image resize_bilinear_omp(const Image& in, int out_w, int out_h, int threads) {
    Image out(out_w, out_h, in.channels);

#if HAVE_OPENMP
    if (threads > 0) omp_set_num_threads(threads);

    #pragma omp parallel for schedule(static)
    for (int y = 0; y < out_h; ++y) {
        const float sy = map_coord(static_cast<float>(y), static_cast<float>(in.height), static_cast<float>(out_h));
        const int y0 = clamp_int(static_cast<int>(std::floor(sy)), 0, in.height - 1);
        const int y1 = clamp_int(y0 + 1, 0, in.height - 1);
        const float wy = sy - static_cast<float>(y0);

        const std::uint8_t* row0 = in.row_ptr(y0);
        const std::uint8_t* row1 = in.row_ptr(y1);
        std::uint8_t* dst_row = out.row_ptr(y);

        for (int x = 0; x < out_w; ++x) {
            const float sx = map_coord(static_cast<float>(x), static_cast<float>(in.width), static_cast<float>(out_w));
            const int x0 = clamp_int(static_cast<int>(std::floor(sx)), 0, in.width - 1);
            const int x1 = clamp_int(x0 + 1, 0, in.width - 1);
            const float wx = sx - static_cast<float>(x0);

            const std::uint8_t* p00 = row0 + (x0 * in.channels);
            const std::uint8_t* p10 = row0 + (x1 * in.channels);
            const std::uint8_t* p01 = row1 + (x0 * in.channels);
            const std::uint8_t* p11 = row1 + (x1 * in.channels);

            std::uint8_t* dst = dst_row + (x * out.channels);

            for (int c = 0; c < out.channels; ++c) {
                const float v00 = static_cast<float>(p00[c]);
                const float v10 = static_cast<float>(p10[c]);
                const float v01 = static_cast<float>(p01[c]);
                const float v11 = static_cast<float>(p11[c]);

                const float v0 = v00 + wx * (v10 - v00);
                const float v1 = v01 + wx * (v11 - v01);
                const float v  = v0  + wy * (v1  - v0);

                dst[c] = clamp_u8(static_cast<int>(std::lround(v)));
            }
        }
    }
#else
    (void)threads;
    out = resize_seq(in, out_w, out_h, ResizeMethod::Bilinear);
#endif

    return out;
}

Image resize_omp(const Image& in, int out_w, int out_h, ResizeMethod method, int threads) {
    if (in.empty()) throw std::invalid_argument("resize_omp: input image is empty");
    if (out_w <= 0 || out_h <= 0) throw std::invalid_argument("resize_omp: output size must be > 0");

#if !HAVE_OPENMP
    // compila comunque, ma “degrada” a sequenziale
    return resize_seq(in, out_w, out_h, method);
#else
    switch (method) {
        case ResizeMethod::Nearest:  return resize_nearest_omp(in, out_w, out_h, threads);
        case ResizeMethod::Bilinear: return resize_bilinear_omp(in, out_w, out_h, threads);
        default: throw std::invalid_argument("resize_omp: unsupported method");
    }
#endif
}
