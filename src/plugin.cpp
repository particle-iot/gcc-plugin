/*
 * Copyright (C) 2017 Particle Industries, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

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
    // '*free_lang_data' is an IPA pass executed immediately after '*build_cgraph_edges', which is
    // the pass that builds the callgraph
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
