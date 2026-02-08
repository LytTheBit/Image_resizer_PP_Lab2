// util.hpp
// Created by Francesco on 08/02/2026.
//
// Small utilities used across the project.
// Provides string normalization and robust numeric parsing helpers for CLI.
#pragma once

#include <string>
#include <string_view>

std::string to_lower(std::string s);

bool ends_with_icase(std::string_view s, std::string_view suffix);

int parse_int(std::string_view s, std::string_view name);
