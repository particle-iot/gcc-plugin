#pragma once

#include <functional>
#include <memory>
#include <string>
#include <cstdint>
#include <cassert>

// GCC 5.4.x: Including this header helps to avoid some weird global name conflicts when the
// plugin API is used in C++ code
#include <ios>

namespace particle {

using namespace std::placeholders;

} // namespace particle
