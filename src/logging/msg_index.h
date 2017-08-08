#pragma once

#include "common.h"

#include <unordered_map>
#include <vector>

namespace particle {

typedef unsigned MsgId;

const MsgId INVALID_MSG_ID = 0;

class MsgIndex {
public:
    explicit MsgIndex(const std::string& destFile, const std::string& predefFile = std::string());

    template<typename IterT, typename MsgStrT, typename AssignIdT>
    void process(IterT begin, IterT end, MsgStrT msgStr, AssignIdT assignId, size_t sizeHint = 0);

    template<typename MsgsT, typename MsgStrT, typename AssignIdT>
    void process(const MsgsT& msgs, MsgStrT msgStr, AssignIdT assignId);

private:
    struct Msg {
        MsgId id; // Message ID
        void* data; // User data

        explicit Msg(void* data) :
                id(INVALID_MSG_ID),
                data(data) {
        }
    };

    typedef std::unordered_map<std::string, Msg> MsgMap;

    class IndexReader;
    class IndexWriter;

    std::string destFile_, predefFile_;

    void process(MsgMap* msgMap);

    friend class IndexReader;
    friend class IndexWriter;
};

} // namespace particle

template<typename IterT, typename MsgStrT, typename AssignIdT>
inline void particle::MsgIndex::process(IterT begin, IterT end, MsgStrT msgStr, AssignIdT assignId, size_t sizeHint) {
    // Copy all messages to internal map
    MsgMap msgMap;
    std::vector<IterT> srcIts;
    srcIts.reserve(sizeHint);
    for (auto it = begin; it != end; ++it) {
        srcIts.push_back(it);
        const std::string str = msgStr(it);
        msgMap.insert(std::make_pair(str, Msg(&srcIts.back())));
    }
    // Process messages
    process(&msgMap);
    // Invoke user handler for each message
    for (auto it = msgMap.begin(); it != msgMap.end(); ++it) {
        auto srcIt = static_cast<IterT*>(it->second.data);
        assignId(*srcIt, it->second.id);
    }
}

template<typename MsgsT, typename MsgStrT, typename AssignIdT>
inline void particle::MsgIndex::process(const MsgsT& msgs, MsgStrT msgStr, AssignIdT assignId) {
    process(msgs.begin(), msgs.end(), msgStr, assignId, msgs.size() /* sizeHint */);
}
