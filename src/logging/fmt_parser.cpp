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

#include "logging/fmt_parser.h"

#include "error.h"
#include "debug.h"

#include <boost/algorithm/string/join.hpp>

#include <cctype>

namespace {

using namespace particle;

class InvalidFmtStrError: public Error {
public:
    using Error::Error;
};

inline bool isOneOf(char c, const char* s) {
    while (*s != '\0') {
        if (*s == c) {
            return true;
        }
        ++s;
    }
    return false;
}

inline char next(const char*& s) {
    const char c = *s;
    if (c == '\0') {
        throw InvalidFmtStrError();
    }
    ++s;
    return c;
}

particle::FmtParser::Specs parseFmtStr(const std::string& fmt) {
    FmtParser::Specs specs;
    specs.reserve(4);
    // printf() format strings can easily be parsed by a regex, but let's make it in a more readable way
    const char* s = fmt.data();
    char c = 0;
    while ((c = *s) != '\0') {
        if (c != '%') {
            ++s;
            continue;
        }
        const char* const spec = s; // Beginning of the format specifier
        ++s;
        c = next(s);
        // Flags (optional)
        while (isOneOf(c, " -+#0")) {
            c = next(s);
        }
        // Field width (optional)
        if (c == '*') {
            c = next(s);
        } else {
            while (std::isdigit(c)) {
                c = next(s);
            }
        }
        // Precision (optional)
        if (c == '.') {
            c = next(s);
            if (c == '*') {
                c = next(s);
            } else {
                while (std::isdigit(c)) {
                    c = next(s);
                }
            }
        }
        // Length modifier (optional)
        if (c == 'h') {
            c = next(s);
            if (c == 'h') { // hh
                c = next(s);
            }
        } else if (c == 'l') {
            c = next(s);
            if (c == 'l') { // ll
                c = next(s);
            }
        } else if (isOneOf(c, "jztL")) {
            c = next(s);
        }
        // Conversion specifier
        if (c == '%' && s - spec == 2) { // %%
            continue;
        }
        if (!isOneOf(c, "csdioxXufFeEaAgGnp")) {
            throw InvalidFmtStrError();
        }
        specs.push_back(std::string(spec, s - spec));
    }
    return specs;
}

} // namespace

particle::FmtParser::FmtParser(const std::string& fmt) :
        ok_(false) {
    try {
        specs_ = parseFmtStr(fmt);
        ok_ = true;
    } catch (const InvalidFmtStrError&) {
        // This class doesn't throw exceptions in case of parsing errors
    }
}

std::string particle::FmtParser::joinSpecs(const std::string& sep) const {
    return boost::join(specs_, sep);
}
