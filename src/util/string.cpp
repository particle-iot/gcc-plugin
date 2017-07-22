#include "util/string.h"

#include <cstdarg>

namespace {

const size_t INITIAL_FORMAT_STRING_SIZE = 127;

} // namespace

std::string particle::util::format(const char* fmt, ...) {
    std::string s(INITIAL_FORMAT_STRING_SIZE, '\0');
    va_list args;
    va_start(args, fmt);
    const int n = vsnprintf(&s.front(), s.size() + 1, fmt, args);
    va_end(args);
    if (n < 0) {
        return std::string();
    }
    s.resize(n);
    if (n > (int)INITIAL_FORMAT_STRING_SIZE) {
        va_start(args, fmt);
        vsnprintf(&s.front(), s.size() + 1, fmt, args);
        va_end(args);
    }
    return s;
}
