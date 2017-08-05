#include "json.h"

#include "error.h"

#include <rapidjson/rapidjson.h>

#if RAPIDJSON_MAJOR_VERSION * 1000 + RAPIDJSON_MINOR_VERSION < 1001
#error "RapidJSON >= 1.1.x is required to compile this code"
#endif

#include <rapidjson/reader.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/error/en.h>

namespace {

using namespace particle;

class HandlerAdapter: public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, HandlerAdapter> {
public:
    HandlerAdapter(std::istream* strm, JsonReader::Handler* handler) :
            strm_(strm),
            handler_(handler) {
    }

    bool Null() {
        handler_->value(Variant());
        return true;
    }

    bool Bool(bool val) {
        handler_->value(val);
        return true;
    }

    bool Int(int val) {
        handler_->value(val);
        return true;
    }

    bool Uint(unsigned val) {
        handler_->value(val);
        return true;
    }

    bool Double(double val) {
        handler_->value(val);
        return true;
    }

    bool String(const char* str, rapidjson::SizeType size, bool /* copy */) {
        handler_->value(std::string(str, size));
        return true;
    }

    bool StartObject() {
        handler_->beginObject();
        return true;
    }

    bool Key(const char* str, rapidjson::SizeType size, bool /* copy */) {
        handler_->name(std::string(str, size));
        return true;
    }

    bool EndObject(rapidjson::SizeType /* memberCount */) {
        handler_->endObject();
        return true;
    }

    bool StartArray() {
        handler_->beginArray();
        return true;
    }

    bool EndArray(rapidjson::SizeType /* elementCount */) {
        handler_->endArray();
        return true;
    }

private:
    std::istream* strm_;
    JsonReader::Handler* handler_;
};

} // namespace

void particle::JsonReader::parse(std::istream* strm, Handler* handler) {
    HandlerAdapter h(strm, handler); // Wrapper for JsonReader::Handler instance
    rapidjson::IStreamWrapper s(*strm); // Wrapper for std::istream instance
    rapidjson::Reader reader;
    constexpr unsigned flags = rapidjson::kParseCommentsFlag; // Allow comments
    const rapidjson::ParseResult r = reader.Parse<flags>(s, h);
    if (!r) {
        const auto code = r.Code();
        if (code != rapidjson::kParseErrorDocumentEmpty) { // Empty document is not an error
            throw Error("Unable to parse JSON: %s", rapidjson::GetParseError_En(code));
        }
    }
}
