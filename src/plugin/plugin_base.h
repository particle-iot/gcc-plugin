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

#pragma once

#include "plugin/pass_reg_info.h"
#include "plugin/attr_spec.h"
#include "plugin/gcc.h"
#include "util/variant.h"
#include "util/string.h"
#include "common.h"

#include <vector>
#include <map>

#define PARTICLE_PLUGIN_INIT(_type) \
        extern "C" int plugin_init(plugin_name_args *args, plugin_gcc_version *version) { \
            return ::particle::initPlugin<_type>(args, version); \
        }

namespace particle {

// Plugin arguments
typedef std::map<std::string, Variant> PluginArgs;

// Base class for GCC plugins
class PluginBase {
public:
    PluginBase();
    virtual ~PluginBase();

    // Returns plugin name
    const std::string& pluginName() const;

    // Returns plugin arguments
    const PluginArgs& pluginArgs() const;

    // Produces fatal compiler error
    static void error(const std::string& msg);

    template<typename... ArgsT>
    static void error(const std::string& fmt, ArgsT&&... args);

    // Called internally by PARTICLE_PLUGIN_INIT()
    void init(plugin_name_args *args, plugin_gcc_version *version);

    static PluginBase* instance();

protected:
    virtual void init() = 0;

    // Registers a compiler pass
    void registerPass(opt_pass* pass, const PassRegInfo& info);

    // Registers a plugin attribute
    void registerAttr(const AttrSpec& attr);

    // Defines a preprocessor macro
    void defineMacro(const std::string& name);

    template<typename T>
    void defineMacro(const std::string& name, const T& val);

    // Returns global GCC context instance
    static gcc::context* gccContext();

private:
    struct AttrSpecData {
        attribute_spec gcc;
        AttrSpec::Handler handler;
    };

    std::map<std::string, AttrSpecData> attrSpecs_;
    std::vector<std::string> macros_;

    std::string pluginName_;
    PluginArgs pluginArgs_;

    // Plugin callbacks
    static void registerAttrs(void* gccData, void* userData); // event: PLUGIN_ATTRIBUTES
    static tree attrHandler(tree* node, tree name, tree args, int flags, bool* noAddAttrs);
};

template<typename T>
int initPlugin(plugin_name_args *args, plugin_gcc_version *version);

} // namespace particle

inline const std::string& particle::PluginBase::pluginName() const {
    return pluginName_;
}

inline const particle::PluginArgs& particle::PluginBase::pluginArgs() const {
    return pluginArgs_;
}

inline void particle::PluginBase::error(const std::string& msg) {
    ::error("%s", msg.data());
}

template<typename... ArgsT>
inline void particle::PluginBase::error(const std::string& fmt, ArgsT&&... args) {
    error(format(fmt, std::forward<ArgsT>(args)...));
}

template<typename T>
inline void particle::PluginBase::defineMacro(const std::string& name, const T& val) {
    defineMacro(format("%1%=%2%", name, val));
}

template<typename PluginT>
inline int particle::initPlugin(plugin_name_args *args, plugin_gcc_version *version) {
    try {
        static PluginT plugin;
        plugin.PluginBase::init(args, version);
        return 0;
    } catch (const std::exception& e) {
        PluginBase::error(e.what());
        return -1;
    }
}
