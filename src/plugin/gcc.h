#pragma once

#include "common.h"

#include <gcc-plugin.h>
#include <cp/cp-tree.h>
#include <c-family/c-pragma.h> // For parse_in
#include <plugin-version.h>

#ifndef NDEBUG

#include <print-tree.h>
#include <tree-pretty-print.h>

#endif // !defined(NDEBUG)

namespace particle {

typedef tree Tree; // For better readability

} // namespace particle
