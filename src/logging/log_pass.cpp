#include "logging/log_pass.h"

#include "plugin/plugin_base.h"
#include "plugin/tree.h"
#include "debug.h"

namespace {

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

} // namespace

particle::LogPass::LogPass(gcc::context* ctx) :
        gimple_opt_pass(LOG_PASS_DATA, ctx) {
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
                location_t fnDeclLoc = DECL_SOURCE_LOCATION(fnDecl);
                const auto it = logFuncs_.find(fnDeclLoc);
                if (it == logFuncs_.end()) {
                    continue; // Not a logging function
                }
                const LogFuncInfo& logFunc = it->second;
                // Get number of function arguments
                const int argCount = gimple_call_num_args(stmt);
                if (logFunc.fmtArgIndex >= argCount) {
                    throw Error("Invalid index of the format string argument");
                }
                tree t = gimple_call_arg(stmt, logFunc.fmtArgIndex);
                t = TREE_OPERAND(t, logFunc.fmtArgIndex);
                if (TREE_CODE(t) != STRING_CST) {
                    continue; // Not a string constant
                }
                DEBUG("%s", constStrVal(t));
            }
        }
    } catch (const std::exception& e) {
        PluginBase::error(e.what());
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

void particle::LogPass::attrHandler(tree t, std::vector<Variant> args) {
    if (args.size() != 2) {
        throw Error("Invalid number of attribute arguments");
    }
    LogFuncInfo info;
    info.funcName = std::string(); // FIXME
    info.fmtArgIndex = args.at(0).toInt() - 1; // Convert to zero-based index
    info.attrArgIndex = args.at(1).toInt() - 1;
    if (info.fmtArgIndex < 0 || info.attrArgIndex < 0) {
        throw Error("Invalid argument index");
    }
    const location_t loc = DECL_SOURCE_LOCATION(t);
    logFuncs_[loc] = info;
}
