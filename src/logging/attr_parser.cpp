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

// GCC 5.4.x: This header file conflicts with some of the GCC's macro definitions
#include <regex>

#include "logging/attr_parser.h"

#include "plugin/gcc_defs.h"
#include "util/string.h"
#include "error.h"
#include "debug.h"

#include <boost/algorithm/string.hpp>

#include <list>

namespace particle {

namespace {

std::string getSourceLine(const expanded_location& loc) {
    int len = 0;
    const char* const data = location_get_source_line(loc, &len);
    if (!data || len <= 0) {
        return std::string(); // GCC doesn't seem to cache empty lines
    }
    return std::string(data, len);
}

} // namespace

AttrParser::AttrParser(Location loc) {
    parse(expand_location(loc));
}

void AttrParser::parse(expanded_location loc) {
    // Extract the comments block preceeding the logging statement
    std::list<std::string> lines;
    std::string line = getSourceLine(loc);
    line = line.substr(0, loc.column - 1); // Column numbers are 1-based
    bool multiLine = false;
    bool singleLine = false;
    for (;;) {
        boost::trim(line);
        if (multiLine) {
            const size_t p = line.rfind("/*");
            // Trim fancy comment formatting
            size_t n = (p != std::string::npos) ? p + 2: 0;
            while (n < line.size() && line.at(n) == '*') {
                ++n;
            }
            if (n > 0) {
                line = line.substr(n);
                boost::trim_left(line);
            }
            lines.push_front(line);
            if (p != std::string::npos) {
                break; // End of the comments block
            }
        } else if (singleLine) {
            if (boost::starts_with(line, "//")) {
                // Trim fancy comment formatting
                size_t n = 2;
                while (n < line.size() && line.at(n) == '/') {
                    ++n;
                }
                line = line.substr(n);
                boost::trim_left(line);
                lines.push_front(line);
            } else if (!line.empty()) {
                break; // End of the comments block
            }
        } else {
            if (boost::ends_with(line, "*/")) {
                line.resize(line.size() - 2);
                multiLine = true;
                continue;
            } else if (boost::starts_with(line, "//")) {
                singleLine = true;
                continue;
            } else if (!line.empty()) {
                break; // No comments block found
            }
        }
        // Get the previous line
        if (--loc.line <= 0) { // Line numbers are 1-based
            break; // End of the comments block
        }
        line = getSourceLine(loc);
    }
    // Check if the comments block contains Doxygen-alike named parameters
    for (const std::string& line: lines) {
        if (line.empty() || (line.front() != '@' && line.front() != '\\')) {
            continue;
        }
        static const std::regex ID_REGEX(".id\\s*(\\d+)");
        static const std::regex HINT_REGEX(".hint\\s*(.+)");
        static const std::regex HELP_REGEX(".help\\s*(.+)"); // TODO: Validate the identifier syntax
        std::smatch m;
        if (std::regex_match(line, m, ID_REGEX)) {
            if (msgId_) {
                throw Error("Duplicate attribute: `id`");
            }
            msgId_ = fromStr<MsgId>(m.str(1));
        } else if (std::regex_match(line, m, HINT_REGEX)) {
            if (hintMsg_) {
                throw Error("Duplicate attribute: `hint`");
            }
            hintMsg_ = m.str(1);
        } else if (std::regex_match(line, m, HELP_REGEX)) {
            if (helpId_) {
                throw Error("Duplicate attribute: `help`");
            }
            helpId_ = m.str(1);
        }
    }
}

} // namespace particle
