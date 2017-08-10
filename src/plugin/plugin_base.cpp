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

#include "plugin/plugin_base.h"

#include "plugin/tree.h"
#include "error.h"
#include "debug.h"

extern void register_attribute(const attribute_spec*);

// Required by the plugins spec
int plugin_is_GPL_compatible;

namespace {

using namespace particle;

PluginBase* g_instance = nullptr; // Plugin instance

PluginArgs parsePluginArgs(const plugin_name_args* args) {
    PluginArgs argMap;
    for (int i = 0; i < args->argc; ++i) {
        const plugin_argument& arg = args->argv[i];
        assert(arg.key);
        argMap[arg.key] = arg.value ? Variant(arg.value) : Variant();
    }
    return argMap;
}

} // namespace

particle::PluginBase::PluginBase() {
    assert(!g_instance);
    g_instance = this;
}

particle::PluginBase::~PluginBase() {
    g_instance = nullptr;
}

void particle::PluginBase::init(plugin_name_args *args, plugin_gcc_version *version) {
    // Check version of the host compiler
    assert(version);
    if (!plugin_default_version_check(version, &gcc_version /* Defined in plugin-version.h */)) {
        throw Error("This plugin is built for GCC %s", gcc_version.basever);
    }
    // Parse plugin arguments
    assert(args && args->base_name);
    pluginName_ = args->base_name;
    pluginArgs_ = parsePluginArgs(args);
    // Initialize plugin
    this->init();
    // Register callbacks
    register_callback(pluginName_.data(), PLUGIN_START_UNIT, startUnit, this);
    register_callback(pluginName_.data(), PLUGIN_ATTRIBUTES, registerAttrs, this);
}

particle::PluginBase* particle::PluginBase::instance() {
    return g_instance;
}

void particle::PluginBase::registerPass(opt_pass* pass, const PassRegInfo& info) {
    assert(pass);
    if (info.refPassName().empty()) {
        throw Error("Invalid pass info");
    }
    register_pass_info p = { 0 };
    p.pass = pass;
    p.reference_pass_name = info.refPassName().data();
    p.ref_pass_instance_number = info.refPassInstanceNum();
    p.pos_op = info.pos();
    register_callback(pluginName_.data(), PLUGIN_PASS_MANAGER_SETUP, nullptr, &p);
}

void particle::PluginBase::registerAttr(const AttrSpec& attr) {
    if (attr.name().empty()) {
        throw Error("Invalid attribute name");
    }
    // Note: GCC stores attribute specs by reference
    const auto r = attrSpecs_.insert(std::make_pair(attr.name(), AttrSpecData()));
    if (!r.second) {
        throw Error("Attribute \"%s\" is already registered", attr.name());
    }
    const auto it = r.first;
    AttrSpecData& d = it->second;
    std::memset(&d.gcc, 0, sizeof(d.gcc));
    d.gcc.name = it->first.data();
    d.gcc.min_length = attr.minArgCount();
    d.gcc.max_length = attr.maxArgCount();
    d.gcc.handler = attrHandler; // Plugin API callback
    d.handler = attr.handler(); // User function
}

void particle::PluginBase::defineMacro(const std::string& name) {
    // TODO: Handle duplicate definitions?
    macros_.push_back(name);
}

gcc::context* particle::PluginBase::gccContext() {
    return g; // Defined in context.h
}

void particle::PluginBase::startUnit(void *gccData, void *userData) {
    try {
        // Define plugin macros
        PluginBase* const p = static_cast<PluginBase*>(userData);
        assert(p->macros_.empty() || parse_in);
        for (const std::string& macro: p->macros_) {
            cpp_define(parse_in, macro.data());
        }
        p->macros_.clear(); // Not needed anymore
    } catch (const std::exception& e) {
        error(e.what());
    }
}

void particle::PluginBase::registerAttrs(void *gccData, void *userData) {
    try {
        // Register plugin attributes
        PluginBase* const p = static_cast<PluginBase*>(userData);
        for (const auto& pair: p->attrSpecs_) {
            // TODO: Register C++11 attribute as well
            register_attribute(&pair.second.gcc);
        }
    } catch (const std::exception& e) {
        error(e.what());
    }
}

tree particle::PluginBase::attrHandler(tree* node, tree name, tree args, int flags, bool* noAddAttrs) {
    try {
        PluginBase* const p = PluginBase::instance();
        const char* const attrName = IDENTIFIER_POINTER(name);
        const auto it = p->attrSpecs_.find(attrName);
        if (it != p->attrSpecs_.end() && it->second.handler) {
            // Get attribute arguments
            std::vector<Variant> attrArgs;
            for (tree& t = args; t != NULL_TREE; t = TREE_CHAIN(t)) {
                attrArgs.push_back(constVal(TREE_VALUE(t)));
            }
            assert(node);
            it->second.handler(*node, std::move(attrArgs));
        }
    } catch (const std::exception& e) {
        error(e.what());
    }
    return NULL_TREE;
}
