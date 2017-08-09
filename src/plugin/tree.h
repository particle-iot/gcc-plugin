/*
 * Copyright (C) 2017 Particle Industries, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

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
