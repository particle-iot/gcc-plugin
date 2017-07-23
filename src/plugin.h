#pragma once

#include "plugin/plugin_base.h"
#include "common.h"

namespace particle {

class Plugin: public PluginBase {
protected:
    virtual void init() override;

private:
    void attrHandler(tree t, const std::vector<Variant>& args);
};

} // namespace particle
