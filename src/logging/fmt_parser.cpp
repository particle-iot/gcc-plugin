#include "logging/fmt_parser.h"

#include "error.h"

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

} // namespace

particle::FmtParser::FmtParser(const std::string& fmt, char fmtSpecSep) :
        flags_(0),
        ok_(false) {
    try {
        parse(fmt, fmtSpecSep);
    } catch (const InvalidFmtStrError&) {
        // This class doesn't throw exceptions in case of parsing errors
    }
}

void particle::FmtParser::parse(const std::string& fmt, char fmtSpecSep) {
    specStr_.reserve(fmt.size() / 4);
    const char* src = fmt.data();
    char c = 0;
    while ((c = *src) != '\0') {
        if (c != '%') {
            ++src;
            continue;
        }
        const char* const spec = src; // Beginning of the format specifier
        c = next(src);
        // Flags (optional)
        while (isOneOf(c, " -+#0")) {
            c = next(src);
        }
        // Field width (optional)
        if (c == '*') {
            flags_ |= Flag::DYN_SPEC;
            c = next(src);
        } else {
            while (std::isdigit(c)) {
                c = next(src);
            }
        }
        // Precision (optional)
        if (c == '.') {
            c = next(src);
            if (c == '*') {
                flags_ |= Flag::DYN_SPEC;
                c = next(src);
            } else {
                while (std::isdigit(c)) {
                    c = next(src);
                }
            }
        }
        // Length modifier (optional)
        if (c == 'h') {
            c = next(src);
            if (c == 'h') { // hh
                c = next(src);
            }
        } else if (c == 'l') {
            c = next(src);
            if (c == 'l') { // ll
                c = next(src);
            }
        } else if (isOneOf(c, "jztL")) {
            c = next(src);
        }
        // Conversion specifier
        if (c == '%' && src - spec == 2) { // %%
            continue;
        }
        if (!isOneOf(c, "csdioxXufFeEaAgGnp")) {
            throw InvalidFmtStrError();
        }
        if (!specStr_.empty()) {
            specStr_ += fmtSpecSep;
        }
        specStr_.append(spec, src - spec);
    }
    ok_ = true;
}
