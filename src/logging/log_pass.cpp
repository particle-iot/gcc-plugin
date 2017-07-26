#include "logging/log_pass.h"

#include "debug.h"

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
const std::string LOG_ATTR_ID_FLAG_FIELD = "has_id";

// Returns index of the message attributes argument for a logging function
unsigned funcAttrArgIndex(tree fnDecl) {
    unsigned i = 0;
    tree args = TYPE_ARG_TYPES(TREE_TYPE(fnDecl));
    for (tree arg = args; arg != NULL_TREE; arg = TREE_CHAIN(arg), ++i) {
        tree t = TREE_VALUE(arg);
        if (TREE_CODE(t) != POINTER_TYPE) {
            continue; // Not a pointer type
        }
        t = TREE_TYPE(t);
        if (TREE_CODE(t) != RECORD_TYPE) {
            continue; // Target type is not a struct
        }
        if (TYPE_NAME_STRING(t) == LOG_ATTR_STRUCT) {
            return i;
        }
    }
    throw Error("Logging function is expected to take `struct %s*` argument", LOG_ATTR_STRUCT);
}

// Returns number of arguments taken by a function
unsigned funcArgCount(tree fnDecl) {
    unsigned n = 0;
    tree args = TYPE_ARG_TYPES(TREE_TYPE(fnDecl));
    for (tree arg = args; arg != NULL_TREE; arg = TREE_CHAIN(arg)) {
        ++n;
    }
    return n;
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
                // Get format string argment
                const unsigned argCount = gimple_call_num_args(stmt);
                if (logFunc.fmtArgIndex >= argCount) {
                    continue;
                }
                tree t = gimple_call_arg(stmt, logFunc.fmtArgIndex);
                t = TREE_OPERAND(t, 0);
                if (TREE_CODE(t) != STRING_CST) {
                    continue; // Not a string constant
                }
                DEBUG("%s", constStrVal(t)); // FIXME
            }
        }
    } catch (const TreeError& e) {
        error(treeLocation(e.where()), e.message());
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
        throw Error("Unsupported attribute: %s");
    }
    if (TREE_CODE(t) != FUNCTION_DECL) {
        throw Error("This attribute can be applied only to function declarations");
    }
    if (args.size() != 1) {
        throw Error("Invalid number of attribute arguments");
    }
    const int fmtArgIndex = args.at(0).toInt();
    if (fmtArgIndex == 0 || fmtArgIndex > (int)funcArgCount(t)) {
        throw Error("Invalid index of the format string argument: %d", fmtArgIndex);
    }
    LogFuncInfo logFunc = { 0 };
    logFunc.fmtArgIndex = fmtArgIndex - 1; // Convert to 0-based index
    logFunc.attrArgIndex = funcAttrArgIndex(t);
    DEBUG_PASS("%s: %s: %s()", name, treeLocation(t).toStr(), IDENTIFIER_POINTER(DECL_NAME(t)));
    logFuncs_[DECL_UID(t)] = std::move(logFunc);
}
