#include "logging/msg_index.h"

#include "util/json.h"
#include "error.h"
#include "debug.h"

#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#include <fstream>
#include <sstream>
#include <mutex>
#include <unordered_set>

// Uncomment to enable more debugging output
// #define DEBUG_MSG_INDEX(...) DEBUG(__VA_ARGS__)

#ifndef DEBUG_MSG_INDEX
#define DEBUG_MSG_INDEX(...)
#endif

namespace {

using namespace particle;

namespace ipc = boost::interprocess;
namespace fs = boost::filesystem;

// Some JSON schema definitions
const std::string JSON_MSG_TEXT_ATTR = "msg";
const std::string JSON_MSG_ID_ATTR = "id";
const unsigned JSON_MSG_OBJ_LEVEL = 2;

// Concatenates two serialized non-empty JSON arrays with message descriptions
void appendJsonIndex(std::ostream* strm, const std::string& json) {
    auto p = json.find('{');
    assert(p != std::string::npos && p > 0);
    const auto p2 = json.rfind('\n', p - 1); // Preserve indentation
    if (p2 != std::string::npos) {
        p = p2 + 1;
    }
    strm->write(",\n", 2);
    strm->write(json.data() + p, json.size() - p);
}

} // namespace

class particle::MsgIndex::IndexReader: public JsonReader::Handler {
public:
    IndexReader(std::istream* strm, MsgIndex::MsgMap* msgMap) :
            msgMap_(msgMap),
            strm_(strm),
            state_(State::NEW),
            level_(0),
            lastMsgEndPos_(-1),
            maxMsgId_(INVALID_MSG_ID),
            msgCount_(0) {
    }

    void parse() {
        JsonReader reader(strm_, this);
        reader.parse();
        // Reader remains in the NEW state in case of an empty file
        checkState(State::NEW | State::DONE);
    }

    virtual void beginObject() override {
        checkState(State::MSG_ARRAY | State::SKIP);
        if (state_ == State::MSG_ARRAY) {
            state_ = State::MSG_OBJ;
        }
        ++level_;
    }

    virtual void endObject() override {
        --level_;
        if (state_ == State::MSG_OBJ) {
            if (!msgText_) {
                throw Error("Message is missing `%s` attribute", JSON_MSG_TEXT_ATTR);
            }
            if (!msgId_) {
                throw Error("Message is missing `%s` attribute", JSON_MSG_ID_ATTR);
            }
            // Update message map
            const auto it = msgMap_->find(*msgText_);
            if (it != msgMap_->end()) {
                Msg& msg = it->second;
                if (msg.id == INVALID_MSG_ID) {
                    const auto r = foundMsgIds_.insert(*msgId_);
                    if (!r.second) {
                        throw Error("Duplicate message ID: %u", *msgId_);
                    }
                    msg.id = *msgId_;
                    DEBUG_MSG_INDEX("Found message: \"%s\", ID: %u", it->first, msg.id);
                } else if (msg.id != *msgId_) {
                    throw Error("Conflicting message description, ID: %u", *msgId_);
                }
            }
            if (maxMsgId_ == INVALID_MSG_ID || *msgId_ > maxMsgId_) {
                maxMsgId_ = *msgId_;
            }
            lastMsgEndPos_ = strm_->tellg();
            ++msgCount_;
            msgText_ = boost::none;
            msgId_ = boost::none;
            state_ = State::MSG_ARRAY;
        } else if (level_ == JSON_MSG_OBJ_LEVEL) {
            state_ = State::MSG_OBJ;
        }
    }

    virtual void beginArray() override {
        checkState(State::NEW | State::SKIP);
        if (state_ == State::NEW) {
            state_ = State::MSG_ARRAY;
        }
        ++level_;
    }

    virtual void endArray() override {
        --level_;
        if (level_ == JSON_MSG_OBJ_LEVEL) {
            state_ = State::MSG_OBJ;
        } else if (level_ == 0) {
            state_ = State::DONE;
        }
    }

    virtual void name(std::string name) override {
        checkState(State::MSG_OBJ | State::SKIP);
        if (state_ == State::MSG_OBJ) {
            if (name == JSON_MSG_TEXT_ATTR) {
                state_ = State::MSG_TEXT;
            } else if (name == JSON_MSG_ID_ATTR) {
                state_ = State::MSG_ID;
            } else {
                state_ = State::SKIP;
            }
        }
    }

    virtual void value(Variant val) override {
        checkState(State::MSG_TEXT | State::MSG_ID | State::SKIP);
        if (state_ == State::MSG_TEXT) {
            if (!val.isString()) {
                throw Error("Message's `%s` attribute is not a string", JSON_MSG_TEXT_ATTR);
            }
            msgText_ = val.toString();
            state_ = State::MSG_OBJ;
        } else if (state_ == State::MSG_ID) {
            if (!val.isInt()) {
                throw Error("Message's `%s` attribute is not an integer", JSON_MSG_ID_ATTR);
            }
            const int id = val.toInt();
            if (id <= 0) {
                throw Error("Invalid message ID: %d", id);
            }
            msgId_ = id;
            state_ = State::MSG_OBJ;
        } else if (level_ == JSON_MSG_OBJ_LEVEL) {
            state_ = State::MSG_OBJ;
        }
    }

    std::istream::pos_type lastMsgEndPos() const {
        return lastMsgEndPos_;
    }

    unsigned foundMsgCount() const {
        return foundMsgIds_.size();
    }

    unsigned totalMsgCount() const {
        return msgCount_;
    }

    MsgId maxMsgId() const {
        return maxMsgId_;
    }

private:
    enum State {
        NEW = 0x01,
        MSG_ARRAY = 0x02,
        MSG_OBJ = 0x04,
        MSG_TEXT = 0x08,
        MSG_ID = 0x10,
        SKIP = 0x20,
        DONE = 0x40
    };

    MsgIndex::MsgMap* msgMap_;
    std::istream* strm_;

    boost::optional<std::string> msgText_;
    boost::optional<MsgId> msgId_;

    State state_;
    unsigned level_;

    std::unordered_set<MsgId> foundMsgIds_;
    std::istream::pos_type lastMsgEndPos_;
    MsgId maxMsgId_;
    unsigned msgCount_;

    void checkState(unsigned mask) const {
        if (!(state_ & mask)) {
            throw Error("Invalid format of the message index file");
        }
    }
};

class particle::MsgIndex::IndexWriter {
public:
    IndexWriter(std::ostream* strm, MsgIndex::MsgMap* msgMap, MsgId maxMsgId) :
            writer_(strm),
            msgMap_(msgMap),
            lastMsgId_(maxMsgId),
            msgCount_(0) {
    }

    void serialize() {
        writer_.beginArray();
        for (auto it = msgMap_->begin(); it != msgMap_->end(); ++it) {
            Msg& msg = it->second;
            if (msg.id != INVALID_MSG_ID) {
                continue;
            }
            msg.id = ++lastMsgId_;
            writer_.beginObject();
            writer_.name(JSON_MSG_TEXT_ATTR).value(it->first);
            writer_.name(JSON_MSG_ID_ATTR).value(msg.id);
            writer_.endObject();
            ++msgCount_;
            DEBUG_MSG_INDEX("New message: \"%s\", ID: %u", it->first, msg.id);
        }
        writer_.endArray();
    }

    unsigned writtenMsgCount() const {
        return msgCount_;
    }

    MsgId maxMsgId() const {
        return lastMsgId_;
    }

private:
    JsonWriter writer_;
    MsgIndex::MsgMap* msgMap_;
    MsgId lastMsgId_;
    unsigned msgCount_;
};

particle::MsgIndex::MsgIndex(const std::string& destFile, const std::string& predefFile) {
    assert(!destFile.empty());
    destFile_ = fs::absolute(destFile).string();
    if (!predefFile.empty()) {
        predefFile_ = fs::absolute(predefFile).string();
    }
}

void particle::MsgIndex::process(MsgMap* msgMap) {
    assert(msgMap);
    if (msgMap->empty()) {
        return;
    }
    // Process destination index file
    DEBUG_MSG_INDEX("Opening destination index file: %s", destFile_);
    std::fstream destStrm;
    destStrm.exceptions(std::ios::badbit); // Enable exceptions
    destStrm.open(destFile_, std::ios::in | std::ios::out | std::ios::app | std::ios::binary);
    if (!destStrm.is_open()) {
        throw Error("Unable to open index file: %s", destFile_);
    }
    // TODO: Acquire a sharable lock first
    ipc::file_lock fileLock(destFile_.data());
    const std::lock_guard<ipc::file_lock> lock(fileLock);
    IndexReader destReader(&destStrm, msgMap);
    destReader.parse();
    unsigned foundMsgCount = destReader.foundMsgCount();
    if (foundMsgCount == msgMap->size()) {
        return; // All messages have been processed
    }
    MsgId maxMsgId = destReader.maxMsgId();
    if (!predefFile_.empty()) {
        // Process predefined index file
        DEBUG_MSG_INDEX("Opening predefined index file: %s", predefFile_);
        std::ifstream predefStrm;
        predefStrm.exceptions(std::ios::badbit); // Enable exceptions
        predefStrm.open(predefFile_, std::ios::in | std::ios::binary);
        if (!predefStrm.is_open()) {
            throw Error("Unable to open index file: %s", predefFile_);
        }
        IndexReader predefReader(&predefStrm, msgMap);
        predefReader.parse();
        foundMsgCount += predefReader.foundMsgCount();
        if (foundMsgCount == msgMap->size()) {
            return; // All messages have been processed
        }
        const MsgId predefMaxMsgId = predefReader.maxMsgId();
        if ((predefMaxMsgId != INVALID_MSG_ID) && (maxMsgId == INVALID_MSG_ID || predefMaxMsgId > maxMsgId)) {
            maxMsgId = predefMaxMsgId;
        }
    }
    if (maxMsgId == INVALID_MSG_ID) {
        maxMsgId = 0;
    }
    DEBUG_MSG_INDEX("Updating destination index file");
    // Serialize new message descriptions
    std::ostringstream newStrm;
    newStrm.exceptions(std::ios::badbit); // Enable exceptions
    IndexWriter newWriter(&newStrm, msgMap, maxMsgId);
    newWriter.serialize();
    assert(newWriter.writtenMsgCount() == msgMap->size() - foundMsgCount);
    const std::string newJson = newStrm.str();
    destStrm.clear(); // Clear state flags
    if (destReader.totalMsgCount() == 0) {
        // Truncate destination file
        fs::resize_file(destFile_, 0);
        destStrm.write(newJson.data(), newJson.size());
    } else {
        // Append messages to the destination file
        fs::resize_file(destFile_, destReader.lastMsgEndPos());
        appendJsonIndex(&destStrm, newJson);
    }
    destStrm.write("\n", 1);
}
