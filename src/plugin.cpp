#include "util/string.h"
#include "error.h"
#include "gcc.h"

extern void register_attribute(const attribute_spec*);

// Required by the plugins spec
int plugin_is_GPL_compatible;

namespace {

using namespace particle;

const int PLUGIN_VERSION = 1; // 0.0.1

void fatalError(const std::string& msg) {
    ::error("%s", msg.data());
}

void registerAttrCallback(void *gccData, void *userData) {
    try {
        // Register plugin attributes
        struct ParticleAttrHandler {
            static tree callback(tree *node, tree name, tree args, int flags, bool *noAddAttrs) {
                return NULL_TREE;
            }
        };
        static const attribute_spec particleAttrSpec = {
            "particle", // name
            0, // min_length
            -1, // max_length
            false, // decl_required
            false, // type_required
            false, // function_type_required
            ParticleAttrHandler::callback, // handler
            false // affects_type_identity
        };
        register_attribute(&particleAttrSpec);
        // Define plugin macros
        assert(parse_in);
        cpp_define(parse_in, util::format("PARTICLE_GCC_PLUGIN=%d", PLUGIN_VERSION).data());
    } catch (const std::exception& e) {
        fatalError(e.what());
    }
}

} // namespace

extern "C" int plugin_init(plugin_name_args *args, plugin_gcc_version *version) {
    try {
        // Check version of the host compiler
        if (!plugin_default_version_check(version, &gcc_version /* See plugin-version.h */)) {
            throw Error("This plugin is built for GCC %s", gcc_version.basever);
        }
        // Parse plugin arguments
        assert(args->base_name);
        const char* const pluginName = args->base_name;
        // Register callbacks
        register_callback(pluginName, PLUGIN_ATTRIBUTES, registerAttrCallback, nullptr);
        return 0;
    } catch (const std::exception& e) {
        fatalError(e.what());
        return -1;
    }
}
