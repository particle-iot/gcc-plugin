#pragma once

#include "plugin/pass_info.h"
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

// Base class for GCC plugins
class PluginBase {
public:
    typedef std::map<std::string, Variant> Args; // Plugin arguments

    PluginBase();
    virtual ~PluginBase();

    // Returns plugin name
    const std::string& name() const;

    // Returns plugin arguments
    const Args& args() const;

    // Produces fatal compiler error
    static void error(const std::string& msg);

    template<typename... ArgsT>
    static void error(const std::string& fmt, ArgsT&&... args);

    // Called internally by PARTICLE_PLUGIN_INIT()
    void init(plugin_name_args *args, plugin_gcc_version *version);

    static PluginBase* instance();

protected:
    virtual void init() = 0;

    // Registers a plugin pass
    void registerPass(const PassInfo& info);

    // Registers a plugin attribute
    void registerAttr(const AttrSpec& attr);

    // Defines a preprocessor macro
    void defineMacro(const std::string& name);

    template<typename T>
    void defineMacro(const std::string& name, const T& val);

private:
    struct AttrSpecData {
        attribute_spec gcc;
        AttrSpec::Handler handler;
    };

    std::map<std::string, AttrSpecData> attrSpecs_;
    std::vector<std::string> macros_;

    std::string name_; // Plugin name
    Args args_; // Plugin arguments

    static PluginBase* s_instance;

    static Args parsePluginArgs(const plugin_name_args* args);

    // Plugin callbacks
    static void registerAttrs(void* gccData, void* userData); // event: PLUGIN_ATTRIBUTES
    static tree attrHandler(tree* node, tree name, tree args, int flags, bool* noAddAttrs);
};

template<typename T>
int initPlugin(plugin_name_args *args, plugin_gcc_version *version);

} // namespace particle

inline const std::string& particle::PluginBase::name() const {
    return name_;
}

inline const particle::PluginBase::Args& particle::PluginBase::args() const {
    return args_;
}

inline void particle::PluginBase::error(const std::string& msg) {
    ::error("%s", msg.data());
}

template<typename... ArgsT>
inline void particle::PluginBase::error(const std::string& fmt, ArgsT&&... args) {
    error(format(fmt, std::forward<ArgsT>(args)...));
}

inline particle::PluginBase* particle::PluginBase::instance() {
    return s_instance;
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
