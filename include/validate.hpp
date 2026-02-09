// validate.hpp
// Created by Francesco on 09/02/2026.
//
// Correctness utilities.
// Compares two images and computes simple difference metrics used to validate
// that sequential and OpenMP implementations produce equivalent results.
#pragma once

#include "image.hpp"
#include <cstdint>

struct DiffStats {
    std::uint64_t different_values = 0; // number of channel values that differ
    int max_abs_diff = 0;              // max |a-b| over all channel values (0..255)
};

DiffStats compare_images(const Image& a, const Image& b);
