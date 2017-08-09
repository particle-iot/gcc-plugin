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

    // Returns `true` if the format string contains "dynamic" format specifiers, such as asterisk ('*')
    // specified in place of a field width
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
