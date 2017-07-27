#include "logging/log_pass.h"

#include "plugin/gimple.h"
#include "debug.h"

#include <boost/optional.hpp>

#define DEBUG_PASS_ENABLED 1

#if DEBUG_PASS_ENABLED
#define DEBUG_PASS(...) DEBUG(__VA_ARGS__)
#else
#define DEBUG_PASS(...)
#endif

namespace {

using namespace particle;

const pass_data LOG_PASS_DATA = {
    GIMPLE_PASS, // type
    "particle_log_pass", // name
    OPTGROUP_NONE, // optinfo_flags
    TV_NONE, // tv_id
    PROP_gimple_any, // properties_required
    0, // properties_provided
    0, // properties_destroyed
    0, // todo_flags_start
    0 // todo_flags_finish
};

const std::string LOG_ATTR_STRUCT = "LogAttributes";
const std::string LOG_ATTR_ID_FIELD = "id";
const std::string LOG_ATTR_HAS_ID_FIELD = "has_id";

// build_component_ref()
// build_simple_component_ref()
tree buildComponentRef(tree ref, tree field) {
    assert(ref != NULL_TREE && field != NULL_TREE);
    tree next = TREE_CHAIN(field);
    field = TREE_VALUE(field);
    tree t = build3_loc(UNKNOWN_LOCATION, COMPONENT_REF, TREE_TYPE(field), ref, field, NULL_TREE);
    if (next != NULL_TREE) {
        t = buildComponentRef(t, next);
    }
    return t;
}

tree findFieldDecl(tree structType, const std::string& fieldName) {
    for (tree field = TYPE_FIELDS(structType); field != NULL_TREE; field = TREE_CHAIN(field)) {
        tree name = DECL_NAME(field);
        if (name == NULL_TREE) {
            tree type = TREE_TYPE(field);
            if (RECORD_OR_UNION_TYPE_P(type)) { // Anonymous struct/union
                tree list = findFieldDecl(type, fieldName);
                if (list != NULL_TREE) {
                    return tree_cons(NULL_TREE, field, list);
                }
            }
        } else if (IDENTIFIER_POINTER(name) == fieldName) {
            return tree_cons(NULL_TREE, field, NULL_TREE);
        }
    }
    return NULL_TREE;
}

} // namespace

particle::LogPass::LogPass(gcc::context* ctx) :
        Pass<gimple_opt_pass>(LOG_PASS_DATA, ctx) {
}

unsigned particle::LogPass::execute(function* fn) {
    try {
        // Iterate over all GIMPLE statements in all basic blocks
        basic_block bb;
        FOR_ALL_BB_FN(bb, fn) {
            for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
                gimple stmt = gsi_stmt(gsi);
                if (!is_gimple_call(stmt)) {
                    continue; // Not a function call
                }
                // Get declaration of the called function
                tree fnDecl = gimple_call_fndecl(stmt);
                const auto it = logFuncs_.find(DECL_UID(fnDecl));
                if (it == logFuncs_.end()) {
                    continue; // Not a logging function
                }
                const LogFuncInfo& logFunc = it->second;
                const unsigned argCount = gimple_call_num_args(stmt);
                if (logFunc.fmtArgIndex >= argCount || logFunc.attrArgIndex >= argCount) {
                    warning(location(stmt), "Unexpected number of arguments");
                    continue;
                }
                // Get format string argment
                tree t = gimple_call_arg(stmt, logFunc.fmtArgIndex);
                if (TREE_CODE(t) != ADDR_EXPR || TREE_OPERAND_LENGTH(t) == 0) {
                    continue;
                }
                t = TREE_OPERAND(t, 0);
                if (TREE_CODE(t) != STRING_CST) {
                    continue; // Not a string constant
                }
                // tree fmtStr = t;
                // Get attributes argument
                t = gimple_call_arg(stmt, logFunc.attrArgIndex);
                if (TREE_CODE(t) != ADDR_EXPR || TREE_OPERAND_LENGTH(t) == 0) {
                    continue;
                }
                // Get declaration of the attributes variable
                t = TREE_OPERAND(t, 0);
                if (TREE_CODE(t) != VAR_DECL) {
                    continue;
                }
                tree varDecl = t;
                // LogAttributes::id
                tree lhs = buildComponentRef(varDecl, logFunc.idFieldDecl);
                tree rhs = build_int_cst(integer_type_node, 777); // FIXME
                gimple g = gimple_build_assign(lhs, rhs);
                gsi_insert_before(&gsi, g, GSI_SAME_STMT);
                // LogAttributes::has_id
                lhs = buildComponentRef(varDecl, logFunc.hasIdFieldDecl);
                rhs = build_int_cst(integer_type_node, 1);
                g = gimple_build_assign(lhs, rhs);
                gsi_insert_before(&gsi, g, GSI_SAME_STMT);
            }
        }
    } catch (const PassError& e) {
        error(e.location(), e.message());
    } catch (const std::exception& e) {
        error(e.what());
    }
    return 0; // No additional TODOs
}

bool particle::LogPass::gate(function*) {
    // Run this pass only if there are logging functions declared in current translation unit
    return !logFuncs_.empty();
}

opt_pass* particle::LogPass::clone() {
    return this; // FIXME
}

void particle::LogPass::attrHandler(tree t, const std::string& name, std::vector<Variant> args) {
    if (name != "log_function") {
        throw Error("Unsupported attribute: %s", name);
    }
    if (TREE_CODE(t) != FUNCTION_DECL) {
        throw Error("This attribute can be applied only to function declarations");
    }
    DEBUG_PASS("%s: %s: %s()", name, location(t).str(), declName(t));
    if (args.size() != 1) {
        throw Error("Invalid number of attribute arguments");
    }
    const int fmtArgIndex = args.at(0).toInt() - 1; // Convert to 0-based index
    if (fmtArgIndex < 0) {
        throw Error("Invalid index of the format string argument");
    }
    logFuncs_[DECL_UID(t)] = logFuncInfo(t, fmtArgIndex);
}

particle::LogPass::LogFuncInfo particle::LogPass::logFuncInfo(const_tree fnDecl, unsigned fmtArgIndex) {
    boost::optional<LogFuncInfo> logFunc;
    unsigned argCount = 0;
    for (tree arg = TYPE_ARG_TYPES(TREE_TYPE(fnDecl)); arg != NULL_TREE; arg = TREE_CHAIN(arg), ++argCount) {
        if (logFunc) {
            continue; // Keep counting function arguments
        }
        tree t = TREE_VALUE(arg);
        if (TREE_CODE(t) != POINTER_TYPE) {
            continue; // Not a pointer type
        }
        t = TREE_TYPE(t);
        if (TREE_CODE(t) != RECORD_TYPE) {
            continue; // Target type is not a struct
        }
        if (typeName(t) != LOG_ATTR_STRUCT) {
            continue;
        }
        if (!COMPLETE_TYPE_P(t)) {
            throw Error("`%s` is an incomplete type", LOG_ATTR_STRUCT);
        }
        logFunc = LogFuncInfo();
        logFunc->fmtArgIndex = fmtArgIndex;
        logFunc->attrArgIndex = argCount;
        logFunc->idFieldDecl = findFieldDecl(t, LOG_ATTR_ID_FIELD);
        logFunc->hasIdFieldDecl = findFieldDecl(t, LOG_ATTR_HAS_ID_FIELD);
    }
    if (!logFunc) {
        throw Error("Logging function is expected to take `struct %s*` argument", LOG_ATTR_STRUCT);
    }
    if (logFunc->idFieldDecl == NULL_TREE || logFunc->hasIdFieldDecl == NULL_TREE) {
        throw Error("`struct %s` is missing required fields", LOG_ATTR_STRUCT);
    }
    if (logFunc->fmtArgIndex >= argCount) {
        throw Error("Invalid index of the format string argument");
    }
    return *logFunc;
}
