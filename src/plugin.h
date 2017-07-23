#pragma once

#include "plugin/plugin_base.h"
#include "common.h"

namespace particle {

class Plugin: public PluginBase {
protected:
    virtual void init() override;
    virtual void registerAttrs() override;
    virtual void defineMacros() override;

private:
    void attrHandler(Tree tree, const std::vector<util::Variant>& args);
};

} // namespace particle
