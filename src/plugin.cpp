#include "plugin.h"

#include "util/string.h"
#include "error.h"
#include "debug.h"

namespace {

const int PLUGIN_VERSION = 1; // 0.0.1

} // namespace

PARTICLE_PLUGIN_INIT(particle::Plugin)

particle::Plugin::Plugin() :
        logPass_(g) { // `g` is a global GCC context
}

void particle::Plugin::init() {
    // Define plugin macro
    defineMacro("PARTICLE_GCC_PLUGIN", PLUGIN_VERSION);
    // Register attributes
    registerAttr(AttrSpec()
            .name("particle")
            .minArgCount(1) // Requires at least one argument
            .handler(std::bind(&Plugin::attrHandler, this, _1, _2)));
    // Register compiler passes
    registerPass(&logPass_, PassRegInfo()
            .runAfter("cfg") // Run after a control flow graph is built
            .refPassInstanceNum(1)); // Run once per function
}

void particle::Plugin::attrHandler(tree t, std::vector<Variant> args) {
    if (args.empty()) {
        throw Error("Invalid number of attribute arguments");
    }
    // Use first argument to dispatch this attribute to a proper pass instance
    const std::string name = args.front().toString();
    args.erase(args.begin());
    if (boost::starts_with(name, "log_")) {
        logPass_.attrHandler(t, name, std::move(args));
    } else {
        throw Error("Invalid attribute argument: %s", name);
    }
}
