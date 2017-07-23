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

unsigned particle::LogPass::execute(function* func) {
    try {
        DEBUG("LogPass::execute()");
    } catch (const std::exception& e) {
        PluginBase::error(e.what());
    }
    return 0; // No additional TODOs
}

bool particle::LogPass::gate(function*) {
    // Run this pass only if logging functions are used in current translation unit
    return !logFuncs_.empty();
}

opt_pass* particle::LogPass::clone() {
    DEBUG("LogPass::clone()");
    return this; // FIXME
}

void particle::LogPass::attrHandler(tree t, std::vector<Variant> args) {
    if (args.size() != 2) {
        throw Error("Invalid number of attribute arguments");
    }
    LogFuncInfo info;
    info.funcName = std::string(); // FIXME
    info.fmtArgIndex = args.at(0).toInt();
    info.attrArgIndex = args.at(1).toInt();
    logFuncs_[treeLoc(t)] = info;
}
