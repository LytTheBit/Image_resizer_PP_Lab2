// io.cpp
// Created by Francesco on 07/02/2026.
//
// stb-based image loading/saving.
// Loads images from disk into Image (uint8, row-major). Supports forcing
// output channel count (1/3/4). Saving supports PNG and JPG.
#include "io.hpp"

#include <stdexcept>
#include <sstream>

// stb headers (add them to your repo, e.g. third_party/stb/)
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_STATIC
#include "stb_image_write.h"

static void validate_channels(int c) {
    if (c != 1 && c != 3 && c != 4) {
        throw std::invalid_argument("I/O supports only 1, 3, or 4 channels in the Image structure.");
    }
}

Image load_image(const std::string& path, int requested_channels) {
    if (requested_channels != 0) validate_channels(requested_channels);

    int w = 0, h = 0, c = 0;
    stbi_uc* pixels = stbi_load(path.c_str(), &w, &h, &c, requested_channels);

    if (!pixels) {
        std::ostringstream oss;
        oss << "Failed to load image: " << path << " (stb: " << stbi_failure_reason() << ")";
        throw std::runtime_error(oss.str());
    }

    const int out_c = (requested_channels == 0) ? c : requested_channels;

    // Normalize to supported channels if original is unsupported (e.g., 2 channels).
    int final_c = out_c;
    if (final_c != 1 && final_c != 3 && final_c != 4) {
        // Fallback: expand to RGB
        stbi_image_free(pixels);
        pixels = stbi_load(path.c_str(), &w, &h, &c, 3);
        if (!pixels) {
            std::ostringstream oss;
            oss << "Failed to reload image as RGB: " << path << " (stb: " << stbi_failure_reason() << ")";
            throw std::runtime_error(oss.str());
        }
        final_c = 3;
    }

    Image img(w, h, final_c);
    const size_t nbytes = static_cast<size_t>(w) * static_cast<size_t>(h) * static_cast<size_t>(final_c);
    std::copy(pixels, pixels + nbytes, img.data.begin());

    stbi_image_free(pixels);
    return img;
}

void save_png(const Image& img, const std::string& path, int compression_level) {
    if (img.empty()) throw std::invalid_argument("save_png: image is empty");
    validate_channels(img.channels);

    if (compression_level < 0) compression_level = 0;
    if (compression_level > 9) compression_level = 9;

    stbi_write_png_compression_level = compression_level;

    const int stride = img.width * img.channels;
    const int ok = stbi_write_png(path.c_str(), img.width, img.height, img.channels,
                                  img.data.data(), stride);
    if (!ok) {
        throw std::runtime_error("save_png: failed to write PNG: " + path);
    }
}

void save_jpg(const Image& img, const std::string& path, int quality) {
    if (img.empty()) throw std::invalid_argument("save_jpg: image is empty");
    validate_channels(img.channels);

    // JPG does not support alpha. If channels==4, you should drop alpha externally.
    if (img.channels == 4) {
        throw std::invalid_argument("save_jpg: JPG does not support RGBA. Save as PNG or convert to RGB.");
    }

    if (quality < 1) quality = 1;
    if (quality > 100) quality = 100;

    const int ok = stbi_write_jpg(path.c_str(), img.width, img.height, img.channels,
                                  img.data.data(), quality);
    if (!ok) {
        throw std::runtime_error("save_jpg: failed to write JPG: " + path);
    }
}
