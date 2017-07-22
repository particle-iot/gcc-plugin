#pragma once

#include "common.h"

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

namespace particle {

namespace util {

// Wrapper over boost::format() providing variadic arguments syntax
template<typename... ArgsT>
std::string format(const std::string& fmt, ArgsT&&... args);

// Convenience functions for string conversions
template<typename T>
std::string toString(const T& val);

template<typename T>
T fromString(const std::string& str);

template<typename T>
T fromString(const std::string& str, const T& defVal);

// Internal implementation
namespace detail {

inline std::string format(boost::format& f) {
    return f.str();
}

template<typename T, typename... ArgsT>
inline std::string format(boost::format& f, T&& arg, ArgsT&&... args) {
    return format(f % std::forward<T>(arg), std::forward<ArgsT>(args)...);
}

} // namespace particle::util::detail

} // namespace particle::util

} // namespace particle

template<typename... ArgsT>
inline std::string particle::util::format(const std::string& fmt, ArgsT&&... args) {
    boost::format f(fmt);
    return detail::format(f, std::forward<ArgsT>(args)...);
}

template<typename T>
inline std::string particle::util::toString(const T& val) {
    return boost::lexical_cast<std::string>(val);
}

template<typename T>
inline T particle::util::fromString(const std::string& str) {
    return boost::lexical_cast<T>(str);
}

template<typename T>
inline T particle::util::fromString(const std::string& str, const T& defVal) {
    try {
        return boost::lexical_cast<T>(str);
    } catch (const boost::bad_lexical_cast&) {
        return defVal;
    }
}
