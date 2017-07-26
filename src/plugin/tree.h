#pragma once

#include "plugin/location.h"
#include "plugin/gcc.h"
#include "util/variant.h"
#include "error.h"
#include "common.h"

namespace particle {

// Unique declaration ID
typedef decltype(tree_decl_minimal::uid) DeclUid;

// Exception class for AST processing errors
class TreeError: public Error {
public:
    template<typename... ArgsT>
    explicit TreeError(const_tree t, ArgsT&&... args);

    const_tree where() const;

private:
    const_tree t_;
};

// Return value of a constant node
int64_t constIntVal(const_tree t);
double constRealVal(const_tree t);
std::string constStrVal(const_tree t);
Variant constVal(const_tree t);

// Returns node location
Location treeLocation(const_tree t);

} // namespace particle

template<typename... ArgsT>
inline particle::TreeError::TreeError(const_tree t, ArgsT&&... args) :
        Error(std::forward<ArgsT>(args)...),
        t_(t) {
}

inline const_tree particle::TreeError::where() const {
    return t_;
}
