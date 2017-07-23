#pragma once

#ifndef NDEBUG

#include "util/string.h"
#include "common.h"

#include <iostream>

#define DEBUG(_fmt, ...) \
        do { \
            std::cerr << ::particle::format(_fmt, ##__VA_ARGS__) << std::endl; \
        } while (false)

#else // defined(NDEBUG)

#define DEBUG(_fmt, ...)

#endif
