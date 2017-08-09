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

    explicit MsgIndex(const std::string& curFile, const std::string& predefFile = std::string());

    template<typename IterT, typename MsgT, typename FmtStrT>
    std::list<Result<IterT>> process(IterT begin, IterT end, FmtStrT MsgT::*fmtStr);

private:
    enum MsgType {
        NEW = 0x01,
        CURRENT = 0x02,
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

    std::string curFile_, predefFile_;

    void process(MsgMap* msgMap) const;
};

} // namespace particle

template<typename IterT, typename MsgT, typename FmtStrT>
inline std::list<particle::MsgIndex::Result<IterT>> particle::MsgIndex::process(IterT begin, IterT end, FmtStrT MsgT::*fmtStr) {
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
