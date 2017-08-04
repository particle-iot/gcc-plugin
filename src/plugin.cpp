#include "plugin.h"

#include "util/string.h"
#include "error.h"
#include "debug.h"

namespace {

const int PLUGIN_VERSION = 1; // 0.0.1

} // namespace

PARTICLE_PLUGIN_INIT(particle::Plugin)

void particle::Plugin::init() {
    // Define plugin macro
    defineMacro("PARTICLE_GCC_PLUGIN", PLUGIN_VERSION);
    // Register attributes
    registerAttr(AttrSpec()
            .name("particle")
            .minArgCount(1) // Requires at least one argument
            .handler(std::bind(&Plugin::attrHandler, this, _1, _2)));
    // Register compiler passes
    logPass_.reset(new LogPass(gccContext(), pluginArgs()));
    // The '*free_lang_data' pass is executed immediately after '*build_cgraph_edges', which is the pass
    // that builds the callgraph
    registerPass(logPass_.get(), PassRegInfo()
            .runBefore("*free_lang_data")
            .refPassInstanceNum(1)); // Just in case
}

void particle::Plugin::attrHandler(tree t, std::vector<Variant> args) {
    // Use first argument to dispatch this attribute to a proper pass instance
    assert(!args.empty());
    const std::string name = args.front().toString();
    args.erase(args.begin());
    if (boost::starts_with(name, "log_")) {
        logPass_->attrHandler(t, name, std::move(args));
    } else {
        throw Error("Invalid attribute argument: \"%s\"", name);
    }
}
