#pragma once

#include <stdexcept>
#include <string>
#include <sstream>
#include <filesystem>

#include <fmt/core.h>
#include <fmt/color.h>

#define THROW(msg) \
    throw std::runtime_error(addContextMessage(msg, __FILE__, __LINE__, __func__))

#define LOG(color, msg, ...) \
    do { \
        auto formatted = fmt::format(addContextMessage(msg, __FILE__, __LINE__, __func__), ##__VA_ARGS__); \
        fmt::print(fmt::fg(color), "{}", formatted); \
        std::fflush(stdout); \
    } while (0)



std::string addContextMessage(const std::string& msg, const char* file, int line, const char* func);