#include "logging/fmt_parser.h"

#include "error.h"
#include "debug.h"

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

particle::FmtParser::FmtParser(const std::string& fmt) :
        flags_(0),
        ok_(false) {
    try {
        Specs specs;
        specs.reserve(4);
        unsigned flags = 0;
        parse(fmt, &specs, &flags);
        specs_ = std::move(specs);
        flags_ = flags;
        ok_ = true;
    } catch (const InvalidFmtStrError&) {
        // This class doesn't throw exceptions in case of parsing errors
    }
}

void particle::FmtParser::parse(const std::string& fmt, Specs* specs, unsigned* flags) {
    assert(specs && flags);
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
            *flags |= Flag::DYN_SPEC;
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
                *flags |= Flag::DYN_SPEC;
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
        specs->push_back(std::string(spec, s - spec));
    }
}
