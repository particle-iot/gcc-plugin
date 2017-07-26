#include "plugin/tree.h"

#include "error.h"
#include "debug.h"

namespace {

using namespace particle;

inline void __attribute__((noreturn)) treeCheckFailed(const_tree t, const char* file, int line) {
    throw TreeError(t, "Tree check failed: %s:%d", file, line);
}

} // namespace

#ifdef ENABLE_TREE_CHECKING

// GCC doesn't provide default implementations of these functions (a name mangling issue?)
void tree_contains_struct_check_failed(const_tree t, const enum tree_node_structure_enum, const char* file, int line,
        const char*) {
    treeCheckFailed(t, file, line);
}

void tree_check_failed(const_tree t, const char* file, int line, const char*, ...) {
    treeCheckFailed(t, file, line);
}

void tree_class_check_failed(const_tree t, const enum tree_code_class, const char* file, int line, const char*) {
    treeCheckFailed(t, file, line);
}

void tree_operand_check_failed(int, const_tree t, const char* file, int line, const char*) {
    treeCheckFailed(t, file, line);
}

void tree_int_cst_elt_check_failed(int, int, const char* file, int line, const char*) {
    treeCheckFailed(NULL_TREE, file, line);
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
    switch (TREE_CODE(t)) {
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
        throw TreeError(t, "Unsupported constant type");
    }
}
