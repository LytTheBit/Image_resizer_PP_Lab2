// scaling_attacks.hpp
// Created by Francesco on 08/02/2026.
//
// Scaling-attack utilities.
// Implements a downscale->upscale pipeline and computes simple distortion metrics.
// This is useful to analyze robustness of resizing methods against adversarial scaling artifacts.
#pragma once

#include "image.hpp"
#include "resize.hpp"

struct AttackMetrics {
    double mae = 0.0;        // mean absolute error over all channels
    double rmse = 0.0;       // root mean squared error
    double psnr = 0.0;       // peak signal-to-noise ratio (dB); large value means small error
    int max_abs = 0;         // max absolute difference (0..255)
};

AttackMetrics down_up_metrics(
    const Image& src,
    int down_w, int down_h,
    ResizeMethod down_method,
    ResizeMethod up_method,
    Backend backend,
    int threads
);
