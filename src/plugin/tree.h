#pragma once

#include "plugin/gcc.h"
#include "util/variant.h"
#include "error.h"
#include "common.h"

#ifndef NDEBUG
#define DEBUG_TREE(_t) \
        do { \
            debug_tree(const_cast<tree>(_t)); \
        } while (false)
#else
#define DEBUG_TREE(_tree)
#endif

namespace particle {

// Exception class for AST processing errors
class TreeError: public Error {
public:
    template<typename... ArgsT>
    explicit TreeError(const_tree t, ArgsT&&... args);
};

// Return value of a constant node
int64_t constIntVal(const_tree t);
double constRealVal(const_tree t);
std::string constStrVal(const_tree t);
Variant constVal(const_tree t);

} // namespace particle

template<typename... ArgsT>
inline particle::TreeError::TreeError(const_tree t, ArgsT&&... args) :
        Error(std::forward<ArgsT>(args)...) {
    // TODO: Use GCC's pretty printer to make the dump a part of the exception info
    DEBUG_TREE(t);
}
