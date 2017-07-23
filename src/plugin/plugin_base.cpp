#include "plugin/plugin_base.h"

#include "error.h"
#include "debug.h"

extern void register_attribute(const attribute_spec*);

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

void particle::PluginBase::registerAttr(const AttrSpec& attr) {
    if (attr.name().empty()) {
        throw Error("Invalid attribute name");
    }
    // Note: GCC stores attribute specs by reference
    const auto res = attrSpecs_.insert(std::make_pair(attr.name(), AttrSpecData()));
    if (!res.second) {
        throw Error("Attribute \"%s\" is already registered", attr.name());
    }
    const auto it = res.first;
    AttrSpecData& d = it->second;
    std::memset(&d.gcc, 0, sizeof(d.gcc));
    d.gcc.name = it->first.data();
    d.gcc.min_length = attr.minArgCount();
    d.gcc.max_length = attr.maxArgCount();
    d.gcc.handler = attrHandler; // Plugin API callback
    d.handler = attr.handler(); // User function
    // TODO: Add support for C++11 attributes
    register_attribute(&d.gcc);
}

void particle::PluginBase::defineMacro(const std::string& name) {
    assert(parse_in);
    cpp_define(parse_in, name.data());
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

void particle::PluginBase::registerAttrs(void *gccData, void *userData) {
    try {
        PluginBase* const p = static_cast<PluginBase*>(userData);
        // Register plugin attributes
        p->registerAttrs();
        // Define plugin macros
        p->defineMacros();
    } catch (const std::exception& e) {
        error(e.what());
    }
}

particle::Tree particle::PluginBase::attrHandler(Tree *node, Tree name, Tree args, int flags,
        bool *noAddAttrs) {
    try {
        PluginBase* const p = instance();
        const char* const attrName = IDENTIFIER_POINTER(name);
        const auto it = p->attrSpecs_.find(attrName);
        if (it != p->attrSpecs_.end() && it->second.handler) {
            // Get attribute arguments
            std::vector<util::Variant> attrArgs;
            assert(node);
            it->second.handler(*node, attrArgs);
        }
    } catch (const std::exception& e) {
        error(e.what());
    }
    return NULL_TREE;
}
