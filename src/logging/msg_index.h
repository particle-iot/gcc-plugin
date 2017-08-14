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

#include "common.h"

#include <unordered_map>
#include <list>

namespace particle {

typedef unsigned MsgId;

const MsgId INVALID_MSG_ID = 0;

class MsgIndex {
public:
    // Result of the index lookup for a given message
    template<typename IterT>
    using Result = std::pair<IterT, MsgId>;

    explicit MsgIndex(const std::string& targetFile, const std::string& predefFile = std::string());

    template<typename IterT, typename MsgT, typename FmtStrT>
    std::list<Result<IterT>> process(IterT begin, IterT end, FmtStrT MsgT::*fmtStr);

private:
    enum MsgType {
        NEW = 0x01,
        TARGET = 0x02,
        PREDEF = 0x04
    };

    struct Msg {
        MsgId id;
        MsgType type;
        void* data; // User data

        Msg() :
                id(INVALID_MSG_ID),
                type(MsgType::NEW),
                data(nullptr) {
        }
    };

    typedef std::unordered_map<std::string, Msg> MsgMap;

    class IndexReader;
    class IndexWriter;

    std::string targetFile_, predefFile_;

    void process(MsgMap* msgMap) const;
};

} // namespace particle

template<typename IterT, typename MsgT, typename FmtStrT>
inline std::list<particle::MsgIndex::Result<IterT>> particle::MsgIndex::process(IterT begin, IterT end, FmtStrT MsgT::*fmtStr) {
    // TODO: This code needs refactoring
    MsgMap msgMap;
    std::list<std::list<IterT>> iterLists; // User iterators
    // Copy all messages to internal map
    for (auto it = begin; it != end; ++it) {
        std::string s = *it.*fmtStr;
        const auto r = msgMap.insert(std::make_pair(std::move(s), Msg()));
        Msg& msg = r.first->second;
        if (r.second) {
            iterLists.push_back(std::list<IterT>());
            msg.data = &iterLists.back();
        }
        const auto iters = static_cast<std::list<IterT>*>(msg.data);
        assert(iters);
        iters->push_back(it);
    }
    // Process messages
    process(&msgMap);
    std::list<Result<IterT>> res;
    for (auto it = msgMap.begin(); it != msgMap.end(); ++it) {
        const Msg& msg = it->second;
        const auto iters = static_cast<std::list<IterT>*>(msg.data);
        assert(iters);
        for (auto it = iters->begin(); it != iters->end(); ++it) {
            res.push_back(std::make_pair(*it, msg.id));
        }
    }
    return res;
}
