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
#include "error.h"
#include "common.h"

#include <boost/optional.hpp>

namespace particle {

class AttrParser {
public:
    class ParsingError;

    AttrParser();
    explicit AttrParser(Location loc);

    void parse(Location loc);

    MsgId msgId() const;
    bool hasMsgId() const;

    std::string hintMsg() const;
    bool hasHintMsg() const;

    std::string helpId() const;
    bool hasHelpId() const;

    bool hasAttrs() const;

private:
    boost::optional<std::string> hintMsg_, helpId_;
    boost::optional<MsgId> msgId_;
};

class AttrParser::ParsingError: public Error {
public:
    ParsingError();

    template<typename... ArgsT>
    explicit ParsingError(ArgsT&&... args);
};

inline AttrParser::AttrParser() {
}

inline AttrParser::AttrParser(Location loc) {
    parse(loc);
}

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

inline AttrParser::ParsingError::ParsingError() :
        ParsingError("Parsing error") {
}

template<typename... ArgsT>
inline AttrParser::ParsingError::ParsingError(ArgsT&&... args) :
        Error(std::forward<ArgsT>(args)...) {
}

} // namespace particle
