#pragma once

#include "util/variant.h"
#include "common.h"

#include <istream>

namespace particle {

// SAX-alike parser for JSON data
class JsonReader {
public:
    class Handler;

    explicit JsonReader(std::istream* strm, Handler* handler = nullptr);

    void parse();

private:
    std::istream* strm_;
    Handler* handler_;
};

class JsonReader::Handler {
public:
    virtual ~Handler() = default;

    virtual void beginObject();
    virtual void endObject();
    virtual void beginArray();
    virtual void endArray();
    virtual void name(std::string name);
    virtual void value(Variant val);
};

} // namespace particle

inline particle::JsonReader::JsonReader(std::istream* strm, Handler* handler) :
        strm_(strm),
        handler_(handler) {
}

inline void particle::JsonReader::Handler::beginObject() {
    // Default implementation does nothing
}

inline void particle::JsonReader::Handler::endObject() {
    // ditto
}

inline void particle::JsonReader::Handler::beginArray() {
    // ditto
}

inline void particle::JsonReader::Handler::endArray() {
    // ditto
}

inline void particle::JsonReader::Handler::name(std::string name) {
    // ditto
}

inline void particle::JsonReader::Handler::value(Variant val) {
    // ditto
}
