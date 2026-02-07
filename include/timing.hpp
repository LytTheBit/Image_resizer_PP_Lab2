// timing.hpp
// Created by Francesco on 07/02/2026.
//
// Provides lightweight timing utilities based on std::chrono.
// Used to measure execution time of resize operations during benchmarking
// while minimizing measurement overhead.

#pragma once

#include <chrono>

inline double now_ms() {
    using clock = std::chrono::high_resolution_clock;
    return std::chrono::duration<double, std::milli>(clock::now().time_since_epoch()).count();
}

template <class F>
double time_ms(F&& f) {
    const double t0 = now_ms();
    f();
    const double t1 = now_ms();
    return t1 - t0;
}
