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

#include "logging/msg_index.h"
#include "plugin/location.h"
#include "common.h"

#include <boost/optional.hpp>

namespace particle {

class AttrParser {
public:
    explicit AttrParser(Location loc);

    MsgId msgId() const;
    bool hasMsgId() const;

    std::string hintMsg() const;
    bool hasHintMsg() const;

    std::string helpId() const;
    bool hasHelpId() const;

    bool hasAttrs() const;

private:
    boost::optional<MsgId> msgId_;
    boost::optional<std::string> hintMsg_;
    boost::optional<std::string> helpId_;

    void parse(expanded_location loc);
};

inline MsgId AttrParser::msgId() const {
    return (msgId_ ? *msgId_ : INVALID_MSG_ID);
}

inline bool AttrParser::hasMsgId() const {
    return (bool)msgId_;
}

inline std::string AttrParser::hintMsg() const {
    return (hintMsg_ ? *hintMsg_ : std::string());
}

inline bool AttrParser::hasHintMsg() const {
    return (bool)hintMsg_;
}

inline std::string AttrParser::helpId() const {
    return (helpId_ ? *helpId_ : std::string());
}

inline bool AttrParser::hasHelpId() const {
    return (bool)helpId_;
}

inline bool AttrParser::hasAttrs() const {
    return (msgId_ || hintMsg_ || helpId_);
}

} // namespace particle
