#pragma once

#include "common.h"

#include <gcc-plugin.h>
#include <cp/cp-tree.h>
#include <c-family/c-pragma.h> // For parse_in
#include <basic-block.h>
#include <tree-ssa-alias.h>
#include <gimple-expr.h>
#include <gimple.h>
#include <gimple-iterator.h>
#include <tree-pass.h>
#include <cgraph.h>
#include <context.h>
#include <plugin-version.h>

#ifndef NDEBUG

#include <print-tree.h>
#include <tree-pretty-print.h>
#include <gimple-pretty-print.h>

#endif // !defined(NDEBUG)
