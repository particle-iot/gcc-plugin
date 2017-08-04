#pragma once

#include "logging/log_pass.h"
#include "plugin/plugin_base.h"
#include "common.h"

namespace particle {

class Plugin: public PluginBase {
protected:
    virtual void init() override;

private:
    std::unique_ptr<LogPass> logPass_;

    void attrHandler(tree t, std::vector<Variant> args);
};

} // namespace particle
