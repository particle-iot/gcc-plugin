#pragma once

#include "plugin/gcc.h"
#include "common.h"

namespace particle {

class Location {
public:
    Location(); // Constructs an invalid location
    explicit Location(location_t loc);

    const std::string& file() const;
    int line() const;
    int column() const;

    bool valid() const;

private:
    std::string file_;
    int line_, col_;
};

} // namespace particle

inline particle::Location::Location() :
        line_(0),
        col_(0) {
}

inline const std::string& particle::Location::file() const {
    return file_;
}

inline int particle::Location::line() const {
    return line_;
}

inline int particle::Location::column() const {
    return col_;
}

inline bool particle::Location::valid() const {
    return (!file_.empty() && line_ > 0 && col_ > 0);
}
