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

#include "common.h"

#include <vector>

namespace particle {

// Parser for printf() format strings
class FmtParser {
public:
    typedef std::vector<std::string> Specs;

    explicit FmtParser(const std::string& fmt);

    // Returns all format specifiers of the format string
    const Specs& specs() const;

    // Join all format specifiers into a single string
    std::string joinSpecs(const std::string& sep) const;
    std::string joinSpecs(char sep) const;

    bool hasSpecs() const;

    // Returns `false` in case of a parsing error
    explicit operator bool() const;

private:
    Specs specs_;
    bool ok_;
};

} // namespace particle

inline const particle::FmtParser::Specs& particle::FmtParser::specs() const {
    return specs_;
}

inline std::string particle::FmtParser::joinSpecs(char sep) const {
    return joinSpecs(std::string(1, sep));
}

inline bool particle::FmtParser::hasSpecs() const {
    return !specs_.empty();
}

inline particle::FmtParser::operator bool() const {
    return ok_;
}
