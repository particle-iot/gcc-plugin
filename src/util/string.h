#pragma once

#include "common.h"

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace particle {

// Wrapper over boost::format() providing variadic arguments syntax
template<typename... ArgsT>
std::string format(const std::string& fmt, ArgsT&&... args);

// Convenience functions for string conversions
template<typename T>
std::string toStr(const T& val);

template<typename T>
T fromStr(const std::string& str);

template<typename T>
T fromStr(const std::string& str, const T& defVal);

// Internal implementation
namespace detail {

inline std::string format(boost::format& f) {
    return f.str();
}

template<typename T, typename... ArgsT>
inline std::string format(boost::format& f, T&& arg, ArgsT&&... args) {
    return format(f % std::forward<T>(arg), std::forward<ArgsT>(args)...);
}

} // namespace particle::detail

} // namespace particle

template<typename... ArgsT>
inline std::string particle::format(const std::string& fmt, ArgsT&&... args) {
    boost::format f(fmt);
    return detail::format(f, std::forward<ArgsT>(args)...);
}

template<typename T>
inline std::string particle::toStr(const T& val) {
    return boost::lexical_cast<std::string>(val);
}

template<typename T>
inline T particle::fromStr(const std::string& str) {
    return boost::lexical_cast<T>(str);
}

template<typename T>
inline T particle::fromStr(const std::string& str, const T& defVal) {
    try {
        return boost::lexical_cast<T>(str);
    } catch (const boost::bad_lexical_cast&) {
        return defVal;
    }
}
