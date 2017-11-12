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

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/functional/hash.hpp>

#include <unordered_map>

namespace particle {

namespace fs = boost::filesystem;

typedef unsigned MsgId;

const MsgId INVALID_MSG_ID = 0;

class MsgIndex {
public:
    // Base class for a source message
    class Msg;

    explicit MsgIndex(const std::string& destFile);
    MsgIndex(const std::string& destFile, const std::vector<std::string>& srcFiles);

    // Assigns IDs to the source messages
    template<typename IterT>
    void process(IterT begin, IterT end);

private:
    // Message source
    enum MsgSrc {
        NEW = 0x01, // New message
        DEST = 0x02, // Message from the destination file
        SRC = 0x04 // Message from one of the source files
    };

    struct MsgKey {
        boost::optional<std::string> hintMsg, helpId;
        std::string fmtStr;

        struct Hash {
            size_t operator()(const MsgKey& key) const {
                size_t h = 0;
                boost::hash_combine(h, key.fmtStr);
                boost::hash_combine(h, (key.hintMsg ? *key.hintMsg : std::string()));
                boost::hash_combine(h, (key.helpId ? *key.helpId : std::string()));
                return h;
            }
        };

        struct Equal {
            bool operator()(const MsgKey& key1, const MsgKey& key2) const {
                return (key1.fmtStr == key2.fmtStr && key1.hintMsg == key2.hintMsg && key1.helpId == key2.helpId);
            }
        };
    };

    struct MsgData {
        MsgId id;
        MsgSrc src;
        std::list<Msg*> msgList; // Source messages

        MsgData() :
                id(INVALID_MSG_ID),
                src(MsgSrc::NEW) {
        }
    };

    typedef std::unordered_map<MsgKey, MsgData, MsgKey::Hash, MsgKey::Equal> MsgDataMap;

    class IndexReader;
    class IndexWriter;

    fs::path destFile_;
    std::vector<fs::path> srcFiles_;

    void process(MsgDataMap* msgMap);
};

class MsgIndex::Msg {
public:
    virtual void msgId(MsgId id) = 0; // Sets message ID
    virtual MsgId msgId() const = 0; // Returns message ID
    virtual std::string fmtStr() const = 0; // Returns format string
    virtual std::string hintMsg() const = 0;  // Returns hint message
    virtual std::string helpId() const = 0; // Returns help entry ID
    virtual std::string srcFile() const = 0; // Returns source file name
    virtual unsigned srcLine() const = 0; // Returns source line number
};

inline MsgIndex::MsgIndex(const std::string& destFile) :
        MsgIndex(destFile, std::vector<std::string>()) {
}

template<typename IterT>
inline void MsgIndex::process(IterT begin, IterT end) {
    MsgDataMap msgMap;
    for (auto msg = begin; msg != end; ++msg) {
        MsgKey key;
        key.fmtStr = msg->fmtStr();
        std::string s = msg->hintMsg();
        if (!s.empty()) {
            key.hintMsg = std::move(s);
        }
        s = msg->helpId();
        if (!s.empty()) {
            key.helpId = std::move(s);
        }
        const auto it = msgMap.insert(std::make_pair(std::move(key), MsgData())).first;
        it->second.msgList.push_back(&*msg);
    }
    process(&msgMap);
    for (auto it = msgMap.begin(); it != msgMap.end(); ++it) {
        assert(it->second.id != INVALID_MSG_ID);
        for (Msg* msg: it->second.msgList) {
            msg->msgId(it->second.id);
        }
    }
}

} // namespace particle
