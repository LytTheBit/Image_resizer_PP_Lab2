// io.hpp
// Created by Francesco on 07/02/2026.
//
// Image I/O interface.
// Provides functions to load and save common image formats (PNG/JPG/BMP/TGA).
// The implementation is based on stb_image/stb_image_write and converts data
// into the project's Image structure (contiguous row-major layout).
#pragma once

#include <string>
#include "image.hpp"

Image load_image(const std::string& path, int requested_channels = 0);
/*
 * requested_channels:
 * 0 = keep original channels
 * 1 = force grayscale
 * 3 = force RGB
 * 4 = force RGBA
 */

void save_png(const Image& img, const std::string& path, int compression_level = 3);
void save_jpg(const Image& img, const std::string& path, int quality = 95);
