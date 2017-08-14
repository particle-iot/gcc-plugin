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
#include <cctype>

namespace {

using namespace particle;

namespace ipc = boost::interprocess;
namespace fs = boost::filesystem;

// JSON schema definitions
const std::string JSON_MSG_TEXT_ATTR = "msg";
const std::string JSON_MSG_ID_ATTR = "id";
const unsigned JSON_MSG_OBJ_LEVEL = 2;

// Concatenates two serialized non-empty JSON arrays of message objects
void appendJsonIndex(std::ostream* strm, const std::string& json) {
    auto p = json.find('{');
    assert(p != std::string::npos && p > 0);
    char c = 0;
    while ((c = json.at(p - 1)) != '\n' && std::isspace(c)) { // Preserve indentation
        assert(p > 1);
        --p;
    }
    strm->write(",\n", 2);
    strm->write(json.data() + p, json.size() - p);
}

} // namespace

class particle::MsgIndex::IndexReader: public particle::JsonReader::Handler {
public:
    IndexReader(std::istream* strm, MsgMap* msgMap, MsgType msgType) :
            msgMap_(msgMap),
            msgType_(msgType),
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
                    assert(msg.type == MsgType::NEW);
                    msg.type = msgType_;
                    DEBUG("Found message: \"%s\", ID: %u", it->first, msg.id);
                } else if (msg.id != *msgId_) {
                    throw Error("Conflicting message, ID: %u", *msgId_);
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
            if (id < 0 || id == INVALID_MSG_ID) {
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

    MsgMap* msgMap_;
    MsgType msgType_;
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
            throw Error("Invalid format of the message data");
        }
    }
};

class particle::MsgIndex::IndexWriter {
public:
    IndexWriter(std::ostream* strm, MsgMap* msgMap, MsgId maxMsgId = INVALID_MSG_ID, unsigned msgTypeMask = 0) :
            writer_(strm),
            msgMap_(msgMap),
            maxMsgId_((maxMsgId != INVALID_MSG_ID) ? maxMsgId : 0),
            msgTypeMask_(msgTypeMask),
            msgCount_(0) {
    }

    void serialize() {
        writer_.beginArray();
        for (auto it = msgMap_->begin(); it != msgMap_->end(); ++it) {
            Msg& msg = it->second;
            if (!msgTypeMask_ || (msg.type & msgTypeMask_)) {
                if (msg.id == INVALID_MSG_ID) {
                    msg.id = ++maxMsgId_;
                    DEBUG("New message: \"%s\", ID: %u", it->first, msg.id);
                }
                writer_.beginObject();
                writer_.name(JSON_MSG_ID_ATTR).value(msg.id);
                writer_.name(JSON_MSG_TEXT_ATTR).value(it->first);
                writer_.endObject();
                ++msgCount_;
            }
        }
        writer_.endArray();
    }

    unsigned writtenMsgCount() const {
        return msgCount_;
    }

    MsgId maxMsgId() const {
        return maxMsgId_;
    }

private:
    JsonWriter writer_;
    MsgMap* msgMap_;
    MsgId maxMsgId_;
    unsigned msgTypeMask_, msgCount_;
};

particle::MsgIndex::MsgIndex(const std::string& targetFile, const std::string& predefFile) {
    assert(!targetFile.empty());
    targetFile_ = fs::weakly_canonical(targetFile).string();
    if (!predefFile.empty()) {
        predefFile_ = fs::weakly_canonical(predefFile).string();
    }
}

void particle::MsgIndex::process(MsgMap* msgMap) const {
    assert(msgMap);
    if (msgMap->empty()) {
        return;
    }
    // Process target message file
    DEBUG("Opening target message file: %s", targetFile_);
    std::fstream targetStrm;
    targetStrm.exceptions(std::ios::badbit); // Enable exceptions
    targetStrm.open(targetFile_, std::ios::in | std::ios::out | std::ios::app | std::ios::binary);
    if (!targetStrm.is_open()) {
        throw Error("Unable to open message file: %s", targetFile_);
    }
    targetStrm.seekg(0);
    // TODO: Acquire a sharable lock first
    ipc::file_lock targetLock(targetFile_.data());
    const std::lock_guard<ipc::file_lock> targetLockGuard(targetLock);
    IndexReader targetReader(&targetStrm, msgMap, MsgType::TARGET);
    targetReader.parse();
    if (targetReader.foundMsgCount() == msgMap->size()) {
        return; // All messages have been processed
    }
    MsgId maxMsgId = targetReader.maxMsgId();
    if (!predefFile_.empty()) {
        // Process predefined message file
        DEBUG("Opening predefined message file: %s", predefFile_);
        std::ifstream predefStrm;
        predefStrm.exceptions(std::ios::badbit); // Enable exceptions
        predefStrm.open(predefFile_, std::ios::in | std::ios::binary);
        if (!predefStrm.is_open()) {
            throw Error("Unable to open message file: %s", predefFile_);
        }
        IndexReader predefReader(&predefStrm, msgMap, MsgType::PREDEF);
        predefReader.parse();
        const MsgId predefMaxMsgId = predefReader.maxMsgId();
        if ((predefMaxMsgId != INVALID_MSG_ID) && (maxMsgId == INVALID_MSG_ID || predefMaxMsgId > maxMsgId)) {
            maxMsgId = predefMaxMsgId;
        }
    }
    // Save new and predefined messages to the target file
    DEBUG("Updating target message file");
    std::ostringstream newStrm;
    newStrm.exceptions(std::ios::badbit); // Enable exceptions
    IndexWriter newWriter(&newStrm, msgMap, maxMsgId, MsgType::NEW | MsgType::PREDEF);
    newWriter.serialize();
    assert(newWriter.writtenMsgCount() + targetReader.foundMsgCount() == msgMap->size());
    const std::string newJson = newStrm.str();
    targetStrm.clear(); // Clear state flags
    // TODO: Truncation of the target file can be avoided in most cases
    if (targetReader.totalMsgCount() == 0) {
        fs::resize_file(targetFile_, 0); // Overwrite file
        targetStrm.write(newJson.data(), newJson.size());
    } else {
        fs::resize_file(targetFile_, targetReader.lastMsgEndPos()); // Append to file
        appendJsonIndex(&targetStrm, newJson);
    }
    targetStrm.write("\n", 1);
    targetStrm.close(); // Flush stream before releasing the file lock
}
