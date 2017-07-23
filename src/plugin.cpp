#include "plugin.h"
#include "debug.h"

namespace {

const int PLUGIN_VERSION = 1; // 0.0.1

} // namespace

PARTICLE_PLUGIN_INIT(particle::Plugin)

void particle::Plugin::init() {
}

void particle::Plugin::registerAttrs() {
    registerAttr(AttrSpec()
            .name("particle")
            .handler(std::bind(&Plugin::attrHandler, this, _1, _2)));
}

void particle::Plugin::defineMacros() {
    defineMacro("PARTICLE_GCC_PLUGIN", PLUGIN_VERSION);
}

void particle::Plugin::attrHandler(tree t, const std::vector<Variant>& args) {
}
