#pragma once

#include "plugin/location.h"
#include "plugin/gcc.h"
#include "common.h"

#ifndef NDEBUG
#define DEBUG_GIMPLE(_g) \
        do { \
            debug_gimple_stmt(const_cast<gimple>(_g)); \
        } while (false)
#else
#define DEBUG_GIMPLE(_g)
#endif

namespace particle {

Location location(const_gimple g);

} // namespace particle
