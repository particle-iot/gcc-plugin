#pragma once

#include "util/variant.h"
#include "common.h"

#include <istream>

namespace particle {

class JsonReader;

class JsonReaderHandler {
public:
    JsonReaderHandler& beginObject(std::function<void()> fn);
    JsonReaderHandler& endObject(std::function<void()> fn);
    JsonReaderHandler& beginArray(std::function<void()> fn);
    JsonReaderHandler& endArray(std::function<void()> fn);
    JsonReaderHandler& name(std::function<void(std::string)> fn);
    JsonReaderHandler& value(std::function<void(Variant)> fn);

private:
    std::function<void()> beginObj_, endObj_, beginArray_, endArray_;
    std::function<void(std::string)> name_;
    std::function<void(Variant)> val_;

    friend class JsonReader;
};

// SAX-alike parser for JSON data
class JsonReader {
public:
    typedef std::basic_istream<char> Stream;
    typedef JsonReaderHandler Handler;

    JsonReader(Stream* strm, Handler handler);

    void parse();

private:
    Stream* strm_;
    Handler handler_;
};

} // namespace particle

inline particle::JsonReader::JsonReader(Stream* strm, Handler handler) :
        strm_(strm),
        handler_(std::move(handler)) {
}

inline particle::JsonReaderHandler& particle::JsonReaderHandler::beginObject(std::function<void()> fn) {
    beginObj_ = std::move(fn);
    return *this;
}

inline particle::JsonReaderHandler& particle::JsonReaderHandler::endObject(std::function<void()> fn) {
    endObj_ = std::move(fn);
    return *this;
}

inline particle::JsonReaderHandler& particle::JsonReaderHandler::beginArray(std::function<void()> fn) {
    beginArray_ = std::move(fn);
    return *this;
}

inline particle::JsonReaderHandler& particle::JsonReaderHandler::endArray(std::function<void()> fn) {
    endArray_ = std::move(fn);
    return *this;
}

inline particle::JsonReaderHandler& particle::JsonReaderHandler::name(std::function<void(std::string)> fn) {
    name_ = std::move(fn);
    return *this;
}

inline particle::JsonReaderHandler& particle::JsonReaderHandler::value(std::function<void(Variant)> fn) {
    val_ = std::move(fn);
    return *this;
}
