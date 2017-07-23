#pragma once

#include "plugin/gcc.h"
#include "util/string.h"
#include "common.h"

namespace particle {

class Location {
public:
    Location(); // Constructs an invalid location
    explicit Location(location_t loc);

    std::string file() const;
    int line() const;
    int column() const;

    bool valid() const;

    std::string toString() const;

    bool operator<(const Location& loc) const;

private:
    location_t loc_;
};

} // namespace particle

inline particle::Location::Location() :
        Location(UNKNOWN_LOCATION) {
}

inline particle::Location::Location(location_t loc) :
        loc_(loc) {
}

inline std::string particle::Location::file() const {
    const char* const f = LOCATION_FILE(loc_);
    return (f ? std::string(f) : std::string());
}

inline int particle::Location::line() const {
    return LOCATION_LINE(loc_);
}

inline int particle::Location::column() const {
    return LOCATION_COLUMN(loc_);
}

inline bool particle::Location::valid() const {
    return (loc_ != UNKNOWN_LOCATION);
}

inline std::string particle::Location::toString() const {
    if (!valid()) {
        return std::string();
    }
    return format("%s:%d:%s", file(), line(), column());
}

inline bool particle::Location::operator<(const Location& loc) const {
    return (loc_ < loc.loc_);
}
