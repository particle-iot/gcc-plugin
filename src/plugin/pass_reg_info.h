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

#include "plugin/gcc_defs.h"
#include "common.h"

namespace particle {

class PassRegInfo {
public:
    PassRegInfo();

    PassRegInfo& refPassName(std::string name);
    const std::string& refPassName() const;

    // Set to 0 to run this pass for every invocation of the reference pass (default)
    PassRegInfo& refPassInstanceNum(int n);
    int refPassInstanceNum() const;

    // PASS_POS_INSERT_AFTER: insert after the reference pass
    // PASS_POS_INSERT_BEFORE: insert before the reference pass
    // PASS_POS_REPLACE: replace the reference pass
    PassRegInfo& pos(pass_positioning_ops pos);
    pass_positioning_ops pos() const;

    // Convenience methods
    PassRegInfo& runAfter(std::string passName);
    PassRegInfo& runBefore(std::string passName);
    PassRegInfo& runInsteadOf(std::string passName);

private:
    std::string refPass_;
    pass_positioning_ops pos_;
    int refPassNum_;
};

} // namespace particle

inline particle::PassRegInfo::PassRegInfo() :
        pos_(PASS_POS_INSERT_AFTER),
        refPassNum_(0) {
}

inline particle::PassRegInfo& particle::PassRegInfo::refPassName(std::string name) {
    refPass_ = std::move(name);
    return *this;
}

inline const std::string& particle::PassRegInfo::refPassName() const {
    return refPass_;
}

inline particle::PassRegInfo& particle::PassRegInfo::refPassInstanceNum(int n) {
    refPassNum_ = n;
    return *this;
}

inline int particle::PassRegInfo::refPassInstanceNum() const {
    return refPassNum_;
}

inline particle::PassRegInfo& particle::PassRegInfo::pos(pass_positioning_ops pos) {
    pos_ = pos;
    return *this;
}

inline pass_positioning_ops particle::PassRegInfo::pos() const {
    return pos_;
}

inline particle::PassRegInfo& particle::PassRegInfo::runAfter(std::string passName) {
    return refPassName(std::move(passName)).pos(PASS_POS_INSERT_AFTER);
}

inline particle::PassRegInfo& particle::PassRegInfo::runBefore(std::string passName) {
    return refPassName(std::move(passName)).pos(PASS_POS_INSERT_BEFORE);
}

inline particle::PassRegInfo& particle::PassRegInfo::runInsteadOf(std::string passName) {
    return refPassName(std::move(passName)).pos(PASS_POS_REPLACE);
}
