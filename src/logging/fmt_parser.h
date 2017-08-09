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

    // Returns `true` if the format string contains "dynamic" format specifiers, such as an asterisk ('*')
    // character specified in place of a field width
    bool dynSpec() const;

    explicit operator bool() const;

private:
    enum Flag {
        DYN_SPEC = 0x01
    };

    Specs specs_;
    unsigned flags_;
    bool ok_;

    static void parse(const std::string& fmt, Specs* specs, unsigned* flags);
};

} // namespace particle

inline const particle::FmtParser::Specs& particle::FmtParser::specs() const {
    return specs_;
}

inline bool particle::FmtParser::dynSpec() const {
    return (flags_ & Flag::DYN_SPEC);
}

inline particle::FmtParser::operator bool() const {
    return ok_;
}
