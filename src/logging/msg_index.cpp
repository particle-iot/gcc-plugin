#include "logging/msg_index.h"

#include "util/json.h"
#include "error.h"
#include "debug.h"

#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#include <fstream>
#include <mutex>

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

} // namespace

// Handler for index data in JSON format
class particle::MsgIndex::IndexReader: public JsonReader::Handler {
public:
    IndexReader() :
            msgMap_(nullptr),
            state_(State::NEW),
            level_(0) {
    }

    void parse(std::istream* strm, MsgIndex::MsgMap* msgMap) {
        msgMap_ = msgMap;
        msgText_ = boost::none;
        msgId_ = boost::none;
        state_ = State::NEW;
        level_ = 0;
        JsonReader reader;
        reader.parse(strm, this);
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
            DEBUG("%d: %s", *msgId_, *msgText_); // TODO: Update message map
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

private:
    enum State {
        NEW = 0x01,
        MSG_ARRAY = 0x02,
        MSG_OBJ = 0x04,
        MSG_TEXT = 0x08,
        MSG_ID = 0x10,
        SKIP = 0x20
    };

    MsgIndex::MsgMap* msgMap_;

    boost::optional<std::string> msgText_;
    boost::optional<MsgId> msgId_;

    State state_;
    unsigned level_;

    void checkState(unsigned mask) const {
        if (!(state_ & mask)) {
            throw Error("Invalid format of the message index file");
        }
    }
};

particle::MsgIndex::MsgIndex(const std::string& destIndexFile, const std::string& predefIndexFile) :
        destIndexFile_(fs::absolute(destIndexFile).string()),
        predefIndexFile_(fs::absolute(predefIndexFile).string()) {
}

void particle::MsgIndex::process(MsgMap* msgMap) {
    assert(msgMap);
    if (msgMap->empty()) {
        return;
    }
    // Open destination index file
    DEBUG("Opening index file: %s", destIndexFile_);
    std::fstream destStrm;
    destStrm.exceptions(std::ios_base::failbit | std::ios_base::badbit); // Enable exceptions
    destStrm.open(destIndexFile_, std::ios_base::in | std::ios_base::out | std::ios_base::app);
    // TODO: Acquire a sharable lock first
    ipc::file_lock fileLock(destIndexFile_.data());
    const std::lock_guard<ipc::file_lock> lock(fileLock);
    IndexReader indexReader;
    indexReader.parse(&destStrm, msgMap);
}
