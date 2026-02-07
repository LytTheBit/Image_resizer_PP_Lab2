// image.hpp
// Created by Francesco on 07/02/2026.
//
// Defines the Image data structure and low-level utilities.
// The image is stored in a contiguous row-major layout and supports
// 1, 3, or 4 channels. This file provides safe accessors and
// basic clamping helpers used by all resize backends.

#pragma once

#include <cstdint>
#include <vector>
#include <stdexcept>
#include <string>
#include <algorithm>

struct Image {
    int width  = 0;
    int height = 0;
    int channels = 0; // 1=Gray, 3=RGB, 4=RGBA
    std::vector<std::uint8_t> data; // size = width*height*channels

    Image() = default;

    Image(int w, int h, int c)
        : width(w), height(h), channels(c),
          data(static_cast<size_t>(w) * static_cast<size_t>(h) * static_cast<size_t>(c), 0u) {
        if (w <= 0 || h <= 0) throw std::invalid_argument("Image: width/height must be > 0");
        if (c != 1 && c != 3 && c != 4) throw std::invalid_argument("Image: channels must be 1,3,4");
    }

    [[nodiscard]] bool empty() const noexcept {
        return width <= 0 || height <= 0 || channels <= 0 || data.empty();
    }

    [[nodiscard]] size_t size_bytes() const noexcept { return data.size(); }

    [[nodiscard]] std::uint8_t* row_ptr(int y) {
        return data.data() + static_cast<size_t>(y) * static_cast<size_t>(width) * static_cast<size_t>(channels);
    }
    [[nodiscard]] const std::uint8_t* row_ptr(int y) const {
        return data.data() + static_cast<size_t>(y) * static_cast<size_t>(width) * static_cast<size_t>(channels);
    }

    [[nodiscard]] std::uint8_t& at(int x, int y, int c) {
        return data[(static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)) * static_cast<size_t>(channels)
                    + static_cast<size_t>(c)];
    }
    [[nodiscard]] const std::uint8_t& at(int x, int y, int c) const {
        return data[(static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)) * static_cast<size_t>(channels)
                    + static_cast<size_t>(c)];
    }
};

inline int clamp_int(int v, int lo, int hi) {
    return std::max(lo, std::min(hi, v));
}

inline std::uint8_t clamp_u8(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return static_cast<std::uint8_t>(v);
}
