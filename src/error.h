#pragma once

#include "util/string.h"
#include "common.h"

#include <exception>

namespace particle {

class Error: public std::exception {
public:
    Error();
    explicit Error(std::string msg);
    explicit Error(const char* msg);

    template<typename... ArgsT>
    explicit Error(const char* fmt, ArgsT&&... args);

    const std::string& message() const;

    virtual const char* what() const noexcept override;

private:
    std::string msg_;
};

} // namespace particle

inline particle::Error::Error() {
}

inline particle::Error::Error(std::string msg) :
        msg_(std::move(msg)) {
}

inline particle::Error::Error(const char* msg) :
        msg_(msg) {
}

template<typename... ArgsT>
inline particle::Error::Error(const char* fmt, ArgsT&&... args) :
        msg_(util::format(fmt, std::forward<ArgsT>(args)...)) {
}

inline const std::string& particle::Error::message() const {
    return msg_;
}

inline const char* particle::Error::what() const noexcept {
    return msg_.data();
}
