#include "plugin.h"

namespace {

const int PLUGIN_VERSION = 1; // 0.0.1

} // namespace

void particle::Plugin::init() {
}

void particle::Plugin::defineMacros() {
    // defineMacro("PARTICLE_GCC_PLUGIN", PLUGIN_VERSION);
}

PARTICLE_PLUGIN_INIT(particle::Plugin)
