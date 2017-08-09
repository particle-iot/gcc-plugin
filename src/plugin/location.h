/*
 * Copyright (C) 2017 Particle Industries, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "plugin/gcc.h"
#include "util/string.h"
#include "common.h"

namespace particle {

class Location {
public:
    Location(); // Constructs an invalid location
    Location(location_t loc);

    std::string file() const;
    int line() const;
    int column() const;

    bool valid() const;

    std::string str() const;

    operator location_t() const;

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

inline std::string particle::Location::str() const {
    if (!valid()) {
        return std::string();
    }
    return format("%s:%d:%s", file(), line(), column());
}

inline particle::Location::operator location_t() const {
    return loc_;
}
