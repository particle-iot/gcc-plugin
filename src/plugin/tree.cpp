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

#include "plugin/tree.h"

#include "error.h"
#include "debug.h"

namespace {

using namespace particle;

// Returns class name of a tree node (for debugging purposes)
inline std::string className(const_tree t) {
    return TREE_CODE_CLASS_STRING(TREE_CODE_CLASS(TREE_CODE(t)));
}

inline void __attribute__((noreturn)) treeCheckFailed(const char* file, int line, const_tree t = NULL_TREE) {
    if (t != NULL_TREE) {
        DEBUG_TREE(t);
    }
    throw Error("Tree check failed: %s:%d", file, line);
}

} // namespace

#ifdef ENABLE_TREE_CHECKING

// It's either a name mangling issue or GCC doesn't provide implementations of these functions
void tree_contains_struct_check_failed(const_tree t, const enum tree_node_structure_enum, const char* file, int line,
        const char*) {
    treeCheckFailed(file, line, t);
}

void tree_check_failed(const_tree t, const char* file, int line, const char*, ...) {
    treeCheckFailed(file, line, t);
}

void tree_class_check_failed(const_tree t, const enum tree_code_class, const char* file, int line, const char*) {
    treeCheckFailed(file, line, t);
}

void tree_operand_check_failed(int, const_tree t, const char* file, int line, const char*) {
    treeCheckFailed(file, line, t);
}

void tree_int_cst_elt_check_failed(int, int, const char* file, int line, const char*) {
    treeCheckFailed(file, line);
}

#endif // defined(ENABLE_TREE_CHECKING)

int64_t particle::constIntVal(const_tree t) {
    // FIXME: Is there a way to access constant values using built-in types?
    const_tree type = TREE_TYPE(t);
    char buf[WIDE_INT_PRINT_BUFFER_SIZE];
    print_dec(t, buf, TYPE_SIGN(type));
    return fromStr<int64_t>(buf);
}

double particle::constRealVal(const_tree t) {
    const REAL_VALUE_TYPE* v = TREE_REAL_CST_PTR(t);
    char buf[128]; // FIXME
    real_to_decimal(buf, v, sizeof(buf), 0, 1);
    return fromStr<double>(buf);
}

std::string particle::constStrVal(const_tree t) {
    return TREE_STRING_POINTER(t);
}

particle::Variant particle::constVal(const_tree t) {
    const tree_code code = TREE_CODE(t);
    switch (code) {
    case INTEGER_CST: {
        return Variant((int)constIntVal(t));
    }
    case REAL_CST: {
        return Variant(constRealVal(t));
    }
    case STRING_CST: {
        return Variant(constStrVal(t));
    }
    default:
        throw Error("Unsupported constant type (code: %d)", code);
    }
}

particle::Location particle::location(const_tree t) {
    if (EXPR_P(t)) {
        return EXPR_LOCATION(t);
    } if (DECL_P(t)) {
        return DECL_SOURCE_LOCATION(t);
    } else { // TODO
        throw Error("Unable to get location of the node (class: %s)", className(t));
    }
}

std::string particle::declName(const_tree t) {
    t = DECL_NAME(t);
    if (t == NULL_TREE) {
        return std::string(); // Anonymous declaration
    }
    return IDENTIFIER_POINTER(t);
}

std::string particle::typeName(const_tree t) {
    return IDENTIFIER_POINTER(TYPE_IDENTIFIER(t));
}

bool particle::isConstCharPtr(const_tree t) {
    if (!POINTER_TYPE_P(t)) {
        return false; // Not a pointer type
    }
    return TYPE_STRING_FLAG(TREE_TYPE(t));
}
