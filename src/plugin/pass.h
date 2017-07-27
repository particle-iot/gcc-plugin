#pragma once

#include "plugin/location.h"
#include "plugin/gcc.h"
#include "error.h"
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

// Exception class for pass errors
class PassError: public Error {
public:
    template<typename... ArgsT>
    explicit PassError(Location loc, ArgsT&&... args);

    const Location& location() const;

private:
    Location loc_;
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

template<typename... ArgsT>
inline particle::PassError::PassError(Location loc, ArgsT&&... args) :
        Error(std::forward<ArgsT>(args)...),
        loc_(std::move(loc)) {
}

inline const particle::Location& particle::PassError::location() const {
    return loc_;
}
