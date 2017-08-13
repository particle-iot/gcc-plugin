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

#include "plugin/plugin_base.h"
#include "common.h"

namespace particle {

class LogPass;

class Plugin: public PluginBase {
protected:
    virtual void init() override;

private:
    std::unique_ptr<LogPass> logPass_;

    void attrHandler(tree t, std::vector<Variant> args);
};

} // namespace particle
