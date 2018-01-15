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

#include <fstream>
#include <sstream>
#include <mutex>
#include <unordered_set>
#include <cctype>

namespace ipc = boost::interprocess;

namespace particle {

namespace {

// JSON schema definitions
const std::string JSON_MSG_ID_ATTR = "id";
const std::string JSON_FMT_STR_ATTR = "msg";
const std::string JSON_HINT_MSG_ATTR = "hint";
const std::string JSON_HELP_ID_ATTR = "help";
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

class MsgIndex::IndexReader: public JsonReader::Handler {
public:
    IndexReader(std::istream* strm, MsgDataMap* msgMap, MsgSrc msgSrc) :
            msgMap_(msgMap),
            strm_(strm),
            msgSrc_(msgSrc),
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
            // ID and format string attributes are mandatory
            if (!attrs_.msgId) {
                throw Error("Missing attribute: `%s`", JSON_MSG_ID_ATTR);
            }
            const MsgId msgId = *attrs_.msgId;
            if (!attrs_.fmtStr) {
                throw Error("Missing attribute: `%s`", JSON_FMT_STR_ATTR);
            }
            // Check if there's a source message with the same attributes
            MsgKey key;
            key.fmtStr = *attrs_.fmtStr;
            key.hintMsg = attrs_.hintMsg;
            key.helpId = attrs_.helpId;
            const auto it = msgMap_->find(key);
            if (it != msgMap_->end()) {
                MsgData& data = it->second;
                if (data.id == INVALID_MSG_ID) {
                    const auto r = foundMsgIds_.insert(msgId);
                    if (!r.second) {
                        throw Error("Duplicate message ID: %u", msgId);
                    }
                    data.id = msgId;
                    assert(data.src == MsgSrc::NEW);
                    data.src = msgSrc_;
                    DEBUG("Found message: \"%s\", ID: %u", key.fmtStr, data.id);
                } else if (data.id != msgId) {
                    throw Error("Conflicting message, ID: %u", msgId);
                }
            }
            if (maxMsgId_ == INVALID_MSG_ID || msgId > maxMsgId_) {
                maxMsgId_ = msgId;
            }
            lastMsgEndPos_ = strm_->tellg();
            ++msgCount_;
            attrs_ = Attrs();
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
            if (name == JSON_MSG_ID_ATTR) {
                state_ = State::MSG_ID;
            } else if (name == JSON_FMT_STR_ATTR) {
                state_ = State::FMT_STR;
            } else if (name == JSON_HINT_MSG_ATTR) {
                state_ = State::HINT_MSG;
            } else if (name == JSON_HELP_ID_ATTR) {
                state_ = State::HELP_ID;
            } else {
                state_ = State::SKIP;
            }
        }
    }

    virtual void value(Variant val) override {
        checkState(State::MSG_ID | State::FMT_STR | State::HINT_MSG | State::HELP_ID | State::SKIP);
        if (state_ == State::MSG_ID) {
            checkInt(val, JSON_MSG_ID_ATTR);
            const int msgId = val.toInt();
            if (msgId < 0 || msgId == INVALID_MSG_ID) {
                throw Error("Invalid message ID: %d", msgId);
            }
            attrs_.msgId = msgId;
            state_ = State::MSG_OBJ;
        } else if (state_ == State::FMT_STR) {
            checkString(val, JSON_FMT_STR_ATTR);
            attrs_.fmtStr = val.toString();
            state_ = State::MSG_OBJ;
        } else if (state_ == State::HINT_MSG) {
            checkString(val, JSON_HINT_MSG_ATTR);
            attrs_.hintMsg = val.toString();
            state_ = State::MSG_OBJ;
        } else if (state_ == State::HELP_ID) {
            checkString(val, JSON_HELP_ID_ATTR);
            attrs_.helpId = val.toString();
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
        NEW = 0x0001,
        MSG_ARRAY = 0x0002,
        MSG_OBJ = 0x0004,
        MSG_ID = 0x0008,
        FMT_STR = 0x0010,
        HINT_MSG = 0x0020,
        HELP_ID = 0x0040,
        SKIP = 0x0080,
        DONE = 0x0100
    };

    struct Attrs {
        boost::optional<std::string> fmtStr, hintMsg, helpId;
        boost::optional<MsgId> msgId;
    };

    MsgDataMap* msgMap_;
    std::istream* strm_;
    MsgSrc msgSrc_;

    State state_;
    unsigned level_;

    Attrs attrs_;
    std::unordered_set<MsgId> foundMsgIds_;
    std::istream::pos_type lastMsgEndPos_;
    MsgId maxMsgId_;
    unsigned msgCount_;

    void checkState(unsigned mask) const {
        if (!(state_ & mask)) {
            throw Error("Invalid format of the message data");
        }
    }

    static void checkInt(const Variant& val, const std::string& name) {
        if (!val.isInt()) {
            throw Error("`%s` attribute is not an integer", name);
        }
    }

    static void checkString(const Variant& val, const std::string& name) {
        if (!val.isString()) {
            throw Error("`%s` attribute is not a string", name);
        }
    }
};

class MsgIndex::IndexWriter {
public:
    IndexWriter(std::ostream* strm, MsgDataMap* msgMap, MsgId maxMsgId = INVALID_MSG_ID, unsigned msgSrcMask = 0) :
            writer_(strm),
            msgMap_(msgMap),
            maxMsgId_((maxMsgId != INVALID_MSG_ID) ? maxMsgId : 0),
            msgSrcMask_(msgSrcMask),
            msgCount_(0) {
    }

    void serialize() {
        writer_.beginArray();
        for (auto it = msgMap_->begin(); it != msgMap_->end(); ++it) {
            const MsgKey& key = it->first;
            MsgData& data = it->second;
            if (!msgSrcMask_ || (data.src & msgSrcMask_)) {
                if (data.id == INVALID_MSG_ID) {
                    data.id = ++maxMsgId_;
                    DEBUG("New message: \"%s\", ID: %u", key.fmtStr, data.id);
                }
                writer_.beginObject();
                writer_.name(JSON_MSG_ID_ATTR).value(data.id);
                writer_.name(JSON_FMT_STR_ATTR).value(key.fmtStr);
                if (key.hintMsg) {
                    writer_.name(JSON_HINT_MSG_ATTR).value(*key.hintMsg);
                }
                if (key.helpId) {
                    writer_.name(JSON_HELP_ID_ATTR).value(*key.helpId);
                }
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
    MsgDataMap* msgMap_;
    MsgId maxMsgId_;
    unsigned msgSrcMask_, msgCount_;
};

MsgIndex::MsgIndex(const std::string& destFile, const std::vector<std::string>& srcFiles) {
    // Store absolute paths in order to not depend on directory changes
    assert(!destFile.empty());
    destFile_ = fs::absolute(destFile);
    srcFiles_.reserve(srcFiles.size());
    for (const std::string& srcFile: srcFiles) {
        srcFiles_.push_back(fs::absolute(srcFile));
    }
}

void MsgIndex::process(MsgDataMap* msgMap) {
    assert(msgMap);
    if (msgMap->empty()) {
        return;
    }
    // Ensure destination message file exists
    const fs::path destFilePath(destFile_);
    const std::string destFile = destFilePath.string();
    DEBUG("Opening destination message file: %s", destFile);
    std::fstream destStrm;
    destStrm.exceptions(std::ios::badbit); // Enable exceptions
    destStrm.open(destFile, std::ios::app);
    // TODO: Acquire a sharable lock first
    ipc::file_lock destLock(destFile.data());
    const std::lock_guard<ipc::file_lock> destLockGuard(destLock);
    // Reopen destination file for reading/writing
    destStrm.close();
    destStrm.open(destFile, std::ios::in | std::ios::out | std::ios::binary);
    if (!destStrm.is_open()) {
        throw Error("Unable to open message file: %s", destFile);
    }
    // Process destination file
    IndexReader destReader(&destStrm, msgMap, MsgSrc::DEST);
    destReader.parse();
    if (destReader.foundMsgCount() == msgMap->size()) {
        return; // All messages have been processed
    }
    // Process source message files
    MsgId maxMsgId = destReader.maxMsgId();
    for (const fs::path& srcFilePath: srcFiles_) {
        const std::string srcFile = srcFilePath.string();
        DEBUG("Opening source message file: %s", srcFile);
        std::ifstream srcStrm;
        srcStrm.exceptions(std::ios::badbit); // Enable exceptions
        srcStrm.open(srcFile, std::ios::in | std::ios::binary);
        if (!srcStrm.is_open()) {
            throw Error("Unable to open message file: %s", srcFile);
        }
        IndexReader srcReader(&srcStrm, msgMap, MsgSrc::SRC);
        srcReader.parse();
        const MsgId srcMaxMsgId = srcReader.maxMsgId();
        if ((srcMaxMsgId != INVALID_MSG_ID) && (maxMsgId == INVALID_MSG_ID || srcMaxMsgId > maxMsgId)) {
            maxMsgId = srcMaxMsgId;
        }
    }
    // Save new messages to the destination file
    DEBUG("Updating destination message file");
    std::ostringstream newStrm;
    newStrm.exceptions(std::ios::badbit); // Enable exceptions
    IndexWriter newWriter(&newStrm, msgMap, maxMsgId, MsgSrc::NEW | MsgSrc::SRC);
    newWriter.serialize();
    assert(newWriter.writtenMsgCount() + destReader.foundMsgCount() == msgMap->size());
    const std::string newJson = newStrm.str();
    destStrm.clear(); // Clear state flags
    if (destReader.totalMsgCount() == 0) {
        destStrm.seekp(0); // Overwrite file
        destStrm.write(newJson.data(), newJson.size());
    } else {
        destStrm.seekp(destReader.lastMsgEndPos()); // Append to file
        appendJsonIndex(&destStrm, newJson);
    }
    if (fs::file_size(destFilePath) > (size_t)destStrm.tellp()) {
        fs::resize_file(destFilePath, destStrm.tellp());
    }
    destStrm.write("\n", 1);
    destStrm.close(); // Flush stream before releasing the file lock
}

} // namespace particle
