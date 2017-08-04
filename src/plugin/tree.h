#pragma once

#include "plugin/location.h"
#include "plugin/gcc.h"
#include "util/variant.h"
#include "common.h"

#ifndef NDEBUG
#define DEBUG_TREE(_t) \
        do { \
            debug_tree(const_cast<tree>(_t)); \
        } while (false)
#else
#define DEBUG_TREE(_t)
#endif

namespace particle {

// Unique declaration ID
typedef decltype(tree_decl_minimal::uid) DeclUid;

// Return value of a constant node
int64_t constIntVal(const_tree t);
double constRealVal(const_tree t);
std::string constStrVal(const_tree t);
Variant constVal(const_tree t);

// Returns node location
Location location(const_tree t);

std::string declName(const_tree t);
std::string typeName(const_tree t);

bool isConstCharPtr(const_tree t);

} // namespace particle
