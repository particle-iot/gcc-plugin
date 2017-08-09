#pragma once

#include "common.h"

namespace particle {

// Parser for printf() format strings
class FmtParser {
public:
    // Default separator character
    static const char DEFAULT_FMT_SPEC_SEP = 0x1f; // Unit Separator (US)

    explicit FmtParser(const std::string& fmt, char fmtSpecSep = DEFAULT_FMT_SPEC_SEP);

    // Returns all format specifiers of the format string, separated by `fmtSpecSep` character
    // passed to the constructor
    const std::string& specStr() const;

    // Returns `true` if the format string contains "dynamic" format specifiers, such as asterisk ('*')
    // specified in place of a field width
    bool hasDynSpec() const;

    explicit operator bool() const;

private:
    enum Flag {
        DYN_SPEC = 0x01
    };

    std::string specStr_;
    unsigned flags_;
    bool ok_;

    void parse(const std::string& fmt, char fmtSpecSep);
};

} // namespace particle

inline const std::string& particle::FmtParser::specStr() const {
    return specStr_;
}

inline bool particle::FmtParser::hasDynSpec() const {
    return (flags_ & Flag::DYN_SPEC);
}

inline particle::FmtParser::operator bool() const {
    return ok_;
}
