#include "logging/msg_index.h"

#include "util/json.h"
#include "error.h"
#include "debug.h"

#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/filesystem.hpp>

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

} // namespace

class particle::MsgIndex::IndexReader: public JsonReader::Handler {
public:
    explicit IndexReader(std::istream* strm, MsgIndex::MsgMap* msgMap) :
            jsonReader_(strm, this),
            msgMap_(msgMap) {
    }

    void parse() {
        jsonReader_.parse();
    }

    virtual void beginObject() override {
    }

    virtual void endObject() override {
    }

    virtual void beginArray() override {
    }

    virtual void endArray() override {
    }

    virtual void name(std::string name) override {
    }

    virtual void value(Variant val) override {
    }

private:
    JsonReader jsonReader_;
    MsgIndex::MsgMap* msgMap_;
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
    std::fstream destStrm(destIndexFile_, std::ios_base::in | std::ios_base::out | std::ios_base::app);
    if (!destStrm.is_open()) {
        throw Error("Unable to open file: %s", destIndexFile_);
    }
    // TODO: Acquire a sharable lock first
    ipc::file_lock fileLock(destIndexFile_.data());
    const std::lock_guard<ipc::file_lock> lock(fileLock);
    IndexReader indexReader(&destStrm, msgMap);
    indexReader.parse();
}
