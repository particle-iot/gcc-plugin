#include "plugin/plugin_base.h"

#include "plugin/tree.h"
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

void particle::PluginBase::registerPass(const PassInfo& info) {
    if (!info.pass() || info.refPassName().empty()) {
        throw Error("Invalid pass info");
    }
    register_pass_info p = { 0 };
    p.pass = info.pass();
    p.reference_pass_name = info.refPassName().data();
    p.ref_pass_instance_number = info.refPassInstanceNum();
    p.pos_op = info.pos();
    register_callback(name_.data(), PLUGIN_PASS_MANAGER_SETUP, nullptr, &p);
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

particle::PluginBase::Args particle::PluginBase::parsePluginArgs(const plugin_name_args* args) {
    Args argMap;
    for (int i = 0; i < args->argc; ++i) {
        const plugin_argument& arg = args->argv[i];
        assert(arg.key);
        argMap[arg.key] = arg.value ? Variant(arg.value) : Variant();
    }
    return argMap;
}

void particle::PluginBase::registerAttrs(void *gccData, void *userData) {
    try {
        PluginBase* const p = static_cast<PluginBase*>(userData);
        // Register plugin attributes
        for (const auto& pair: p->attrSpecs_) {
            // TODO: Register C++11 attribute as well
            register_attribute(&pair.second.gcc);
        }
        // Define plugin macros
        assert(p->macros_.empty() || parse_in);
        for (const std::string& macro: p->macros_) {
            cpp_define(parse_in, macro.data());
        }
        p->macros_.clear(); // Not needed anymore
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
