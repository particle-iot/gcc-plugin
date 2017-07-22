#pragma once

#include "common.h"

#include <sstream>

namespace particle {

namespace util {

std::string format(const char* fmt, ...) __attribute__((format(printf, 1, 2)));

std::string toString(int val);
std::string toString(double val);
std::string toString(std::string str); // Dummy conversion

template<typename T>
T fromString(const std::string& str, T defaultVal = T());

template<>
int fromString(const std::string& str, int defaultVal);

template<>
double fromString(const std::string& str, double defaultVal);

template<>
std::string fromString(const std::string& str, std::string defaultVal); // Dummy conversion

} // namespace particle::util

} // namespace particle

inline std::string particle::util::toString(int val) {
    return std::to_string(val);
}

inline std::string particle::util::toString(double val) {
    // Use default precision and formatting
    std::ostringstream s;
    s << val;
    return s.str();
}

inline std::string particle::util::toString(std::string str) {
    return std::move(str);
}

template<>
inline int particle::util::fromString(const std::string& str, int defaultVal) {
    try {
        return std::stoi(str, nullptr, 0);
    } catch (const std::logic_error&) {
        return defaultVal;
    }
}

template<>
inline double particle::util::fromString(const std::string& str, double defaultVal) {
    try {
        return std::stod(str, nullptr);
    } catch (const std::logic_error&) {
        return defaultVal;
    }
}

template<>
inline std::string particle::util::fromString(const std::string& str, std::string defaultVal) {
    return std::move(str);
}
