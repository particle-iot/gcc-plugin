#include "plugin/tree.h"

#include "error.h"

namespace {

inline void __attribute__((noreturn)) treeCheckFailed(const char* file, int line) {
    throw particle::Error("Tree check failed: %s:%d (internal error)", file, line);
}

} // namespace

#ifdef ENABLE_TREE_CHECKING

void tree_contains_struct_check_failed(const_tree, const enum tree_node_structure_enum, const char* file, int line,
        const char*) {
    treeCheckFailed(file, line);
}

void tree_check_failed(const_tree, const char* file, int line, const char*, ...) {
    treeCheckFailed(file, line);
}

#endif // defined(ENABLE_TREE_CHECKING)
