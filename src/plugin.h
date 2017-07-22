#pragma once

#include "plugin/plugin_base.h"
#include "common.h"

namespace particle {

class Plugin: public PluginBase {
protected:
    virtual void init() override;
    virtual void defineMacros() override;
};

} // namespace particle
