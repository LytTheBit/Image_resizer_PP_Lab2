// scaling_attacks.cpp
// Created by Francesco on 08/02/2026.
//
// Implementation of scaling-attack pipeline and metrics.
// Computes MAE/RMSE/PSNR and max absolute difference between original and reconstructed image.
#include "scaling_attacks.hpp"

#include <cmath>
#include <stdexcept>
#include <limits>

static AttackMetrics diff_metrics(const Image& a, const Image& b) {
    if (a.width != b.width || a.height != b.height || a.channels != b.channels) {
        throw std::invalid_argument("diff_metrics: image sizes/channels must match");
    }
    const size_t n = a.data.size();
    if (n == 0) throw std::invalid_argument("diff_metrics: empty images");

    double sum_abs = 0.0;
    double sum_sq = 0.0;
    int max_abs = 0;

    for (size_t i = 0; i < n; ++i) {
        const int da = static_cast<int>(a.data[i]);
        const int db = static_cast<int>(b.data[i]);
        const int d = da - db;
        const int ad = (d < 0) ? -d : d;

        sum_abs += static_cast<double>(ad);
        sum_sq  += static_cast<double>(d) * static_cast<double>(d);
        if (ad > max_abs) max_abs = ad;
    }

    const double mean_abs = sum_abs / static_cast<double>(n);
    const double mse = sum_sq / static_cast<double>(n);
    const double rmse = std::sqrt(mse);

    AttackMetrics m;
    m.mae = mean_abs;
    m.rmse = rmse;
    m.max_abs = max_abs;

    if (mse == 0.0) {
        m.psnr = std::numeric_limits<double>::infinity();
    } else {
        const double peak = 255.0;
        m.psnr = 20.0 * std::log10(peak) - 10.0 * std::log10(mse);
    }

    return m;
}

AttackMetrics down_up_metrics(
    const Image& src,
    int down_w, int down_h,
    ResizeMethod down_method,
    ResizeMethod up_method,
    Backend backend,
    int threads
) {
    if (src.empty()) throw std::invalid_argument("down_up_metrics: empty source image");
    if (down_w <= 0 || down_h <= 0) throw std::invalid_argument("down_up_metrics: invalid downscale size");

    Image down = resize(src, down_w, down_h, down_method, backend, threads);
    Image up   = resize(down, src.width, src.height, up_method, backend, threads);

    return diff_metrics(src, up);
}
