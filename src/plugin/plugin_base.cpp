#include "plugin/plugin_base.h"

#include "error.h"

// Required by the plugins spec
int plugin_is_GPL_compatible;

particle::PluginBase* particle::PluginBase::s_instance = nullptr;

particle::PluginBase::PluginBase() {
    assert(!s_instance);
    s_instance = this;
}

particle::PluginBase::~PluginBase() {
    s_instance = nullptr;
}

void particle::PluginBase::init(plugin_name_args *args, plugin_gcc_version *version) {
    // Check version of the host compiler
    if (!plugin_default_version_check(version, &gcc_version /* Defined in plugin-version.h */)) {
        throw Error("This plugin is built for GCC %s", gcc_version.basever);
    }
    // Parse plugin arguments
    assert(args && args->base_name);
    name_ = args->base_name;
    args_ = parsePluginArgs(args);
    // Initialize plugin
    this->init();
    // Register callbacks
    register_callback(name_.data(), PLUGIN_ATTRIBUTES, registerAttrs, this);
}

void particle::PluginBase::defineMacro(const std::string& name) {
    assert(parse_in);
    cpp_define(parse_in, name.data());
}

void particle::PluginBase::registerAttrs(void *gccData, void *userData) {
    try {
        PluginBase* const p = static_cast<PluginBase*>(userData);
        // Define plugin macros
        p->defineMacros();
    } catch (const std::exception& e) {
        error(e.what());
    }
}

particle::PluginBase::Args particle::PluginBase::parsePluginArgs(const plugin_name_args* args) {
    Args argMap;
    for (int i = 0; i < args->argc; ++i) {
        const plugin_argument& arg = args->argv[i];
        assert(arg.key);
        argMap[arg.key] = arg.value ? util::Variant(arg.value) : util::Variant();
    }
    return argMap;
}
