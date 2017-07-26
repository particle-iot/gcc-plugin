#pragma once

#include "plugin/location.h"
#include "plugin/gcc.h"
#include "util/string.h"
#include "common.h"

namespace particle {

// Mixin for classes implementing a compiler pass
template<typename T>
class Pass: public T {
public:
    template<typename... ArgsT>
    explicit Pass(ArgsT&&... args);

protected:
    template<typename... ArgsT>
    void warning(Location loc, const std::string& fmt, ArgsT&&... args);

    template<typename... ArgsT>
    void error(Location loc, const std::string& fmt, ArgsT&&... args);

    template<typename... ArgsT>
    void error(const std::string& fmt, ArgsT&&... args);
};

} // namespace particle

template<typename T>
template<typename... ArgsT>
inline particle::Pass<T>::Pass(ArgsT&&... args) :
        T(std::forward<ArgsT>(args)...) {
}

template<typename T>
template<typename... ArgsT>
inline void particle::Pass<T>::warning(Location loc, const std::string& fmt, ArgsT&&... args) {
    ::warning_at(loc, 0, "%s", format(fmt, std::forward<ArgsT>(args)...).data());
}

template<typename T>
template<typename... ArgsT>
inline void particle::Pass<T>::error(Location loc, const std::string& fmt, ArgsT&&... args) {
    ::error_at(loc, "%s", format(fmt, std::forward<ArgsT>(args)...).data());
}

template<typename T>
template<typename... ArgsT>
inline void particle::Pass<T>::error(const std::string& fmt, ArgsT&&... args) {
    ::error("%s", format(fmt, std::forward<ArgsT>(args)...).data());
}
