// util.cpp
// Created by Francesco on 08/02/2026.
//
// Implementation of small utilities (string helpers, parsing).
#include "util.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>

std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return s;
}

bool ends_with_icase(std::string_view s, std::string_view suffix) {
    if (suffix.size() > s.size()) return false;
    const auto tail = s.substr(s.size() - suffix.size());
    for (size_t i = 0; i < suffix.size(); ++i) {
        const unsigned char a = static_cast<unsigned char>(tail[i]);
        const unsigned char b = static_cast<unsigned char>(suffix[i]);
        if (std::tolower(a) != std::tolower(b)) return false;
    }
    return true;
}

int parse_int(std::string_view s, std::string_view name) {
    try {
        std::string tmp(s);
        size_t idx = 0;
        int v = std::stoi(tmp, &idx);
        if (idx != tmp.size()) throw std::invalid_argument("trailing chars");
        return v;
    } catch (...) {
        throw std::invalid_argument("Invalid integer for " + std::string(name) + ": " + std::string(s));
    }
}
