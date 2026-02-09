// validate.cpp
// Created by Francesco on 07/02/2026.
//
// Implementation of image comparison metrics for correctness validation.
#include "validate.hpp"
#include <stdexcept>

DiffStats compare_images(const Image& a, const Image& b) {
    if (a.width != b.width || a.height != b.height || a.channels != b.channels) {
        throw std::invalid_argument("compare_images: size/channels mismatch");
    }
    if (a.data.size() != b.data.size()) {
        throw std::invalid_argument("compare_images: buffer size mismatch");
    }

    DiffStats s;
    for (size_t i = 0; i < a.data.size(); ++i) {
        const int da = static_cast<int>(a.data[i]);
        const int db = static_cast<int>(b.data[i]);
        const int d = da - db;
        const int ad = (d < 0) ? -d : d;
        if (ad != 0) s.different_values++;
        if (ad > s.max_abs_diff) s.max_abs_diff = ad;
    }
    return s;
}
