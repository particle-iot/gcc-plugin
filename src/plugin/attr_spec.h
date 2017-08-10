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

#include "plugin/location.h"
#include "plugin/gcc_defs.h"
#include "util/variant.h"
#include "common.h"

#include <vector>

namespace particle {

class AttrSpec {
public:
    // Handler function
    typedef std::function<void(tree, std::vector<Variant>)> Handler;

    AttrSpec(std::string name = std::string());

    AttrSpec& name(std::string name);
    const std::string& name() const;

    AttrSpec& minArgCount(int n);
    int minArgCount() const;

    // Set to -1 if the attribute takes unspecified number of arguments (default)
    AttrSpec& maxArgCount(int n);
    int maxArgCount() const;

    // Sets fixed number of arguments
    AttrSpec& argCount(int n);

    AttrSpec& handler(Handler handler);
    const Handler& handler() const;

private:
    Handler handler_;
    std::string name_;
    int minArgCount_, maxArgCount_;
};

} // namespace particle

inline particle::AttrSpec::AttrSpec(std::string name) :
        name_(std::move(name)),
        minArgCount_(0),
        maxArgCount_(-1) {
}

inline particle::AttrSpec& particle::AttrSpec::name(std::string name) {
    name_ = std::move(name);
    return *this;
}

inline const std::string& particle::AttrSpec::name() const {
    return name_;
}

inline particle::AttrSpec& particle::AttrSpec::minArgCount(int n) {
    minArgCount_ = n;
    return *this;
}

inline int particle::AttrSpec::minArgCount() const {
    return minArgCount_;
}

inline particle::AttrSpec& particle::AttrSpec::maxArgCount(int n) {
    maxArgCount_ = n;
    return *this;
}

inline int particle::AttrSpec::maxArgCount() const {
    return maxArgCount_;
}

inline particle::AttrSpec& particle::AttrSpec::argCount(int n) {
    minArgCount_ = n;
    maxArgCount_ = n;
    return *this;
}

inline particle::AttrSpec& particle::AttrSpec::handler(Handler handler) {
    handler_ = std::move(handler);
    return *this;
}

inline const particle::AttrSpec::Handler& particle::AttrSpec::handler() const {
    return handler_;
}
