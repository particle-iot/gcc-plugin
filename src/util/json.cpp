#include "json.h"

#include "error.h"

#define RAPIDJSON_HAS_STDSTRING 1

#include <rapidjson/rapidjson.h>

#if RAPIDJSON_MAJOR_VERSION * 1000 + RAPIDJSON_MINOR_VERSION < 1001
#error "RapidJSON >= 1.1.x is required to compile this code"
#endif

#include <rapidjson/reader.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/error/en.h>

namespace {

using namespace particle;

namespace json = rapidjson;

class HandlerAdapter: public json::BaseReaderHandler<json::UTF8<>, HandlerAdapter> {
public:
    explicit HandlerAdapter(JsonReader::Handler* handler) :
            h_(handler) {
    }

    bool Null() {
        h_->value(Variant());
        return true;
    }

    bool Bool(bool val) {
        h_->value(val);
        return true;
    }

    bool Int(int val) {
        h_->value(val);
        return true;
    }

    bool Uint(unsigned val) {
        h_->value(val);
        return true;
    }

    bool Double(double val) {
        h_->value(val);
        return true;
    }

    bool String(const char* str, json::SizeType size, bool /* copy */) {
        h_->value(std::string(str, size));
        return true;
    }

    bool StartObject() {
        h_->beginObject();
        return true;
    }

    bool Key(const char* str, json::SizeType size, bool /* copy */) {
        h_->name(std::string(str, size));
        return true;
    }

    bool EndObject(json::SizeType /* memberCount */) {
        h_->endObject();
        return true;
    }

    bool StartArray() {
        h_->beginArray();
        return true;
    }

    bool EndArray(json::SizeType /* elementCount */) {
        h_->endArray();
        return true;
    }

private:
    JsonReader::Handler* h_;
};

} // namespace

void particle::JsonReader::parse(std::istream* strm, Handler* handler) {
    HandlerAdapter h(handler); // Wrapper for JsonReader::Handler instance
    json::IStreamWrapper s(*strm); // Wrapper for std::istream instance
    json::GenericReader<json::UTF8<>, json::UTF8<>> reader;
    constexpr unsigned flags = json::kParseCommentsFlag; // Allow comments
    const json::ParseResult r = reader.Parse<flags>(s, h);
    if (!r) {
        const auto code = r.Code();
        if (code != json::kParseErrorDocumentEmpty) { // Empty document is not an error
            throw Error("Unable to parse JSON: %s", json::GetParseError_En(code));
        }
    }
}

struct particle::JsonWriter::Data {
    json::OStreamWrapper strm;
    json::PrettyWriter<json::OStreamWrapper, json::UTF8<>, json::UTF8<>> writer;

    explicit Data(std::ostream* s) :
            strm(*s),
            writer(strm) {
    }
};

particle::JsonWriter::JsonWriter(std::ostream* strm) :
        d_(new Data(strm)) {
    d_->writer.SetIndent(' ', 2);
}

particle::JsonWriter::~JsonWriter() {
}

particle::JsonWriter& particle::JsonWriter::beginObject() {
    d_->writer.StartObject();
    return *this;
}

particle::JsonWriter& particle::JsonWriter::endObject() {
    d_->writer.EndObject();
    return *this;
}

particle::JsonWriter& particle::JsonWriter::beginArray() {
    d_->writer.StartArray();
    return *this;
}

particle::JsonWriter& particle::JsonWriter::endArray() {
    d_->writer.EndArray();
    return *this;
}

particle::JsonWriter& particle::JsonWriter::name(const std::string& name) {
    d_->writer.Key(name);
    return *this;
}

particle::JsonWriter& particle::JsonWriter::value(const Variant& val) {
    switch (val.type()) {
    case Variant::NONE:
        d_->writer.Null();
        break;
    case Variant::BOOL:
        d_->writer.Bool(val.toBool());
        break;
    case Variant::INT:
        d_->writer.Int(val.toInt());
        break;
    case Variant::DOUBLE:
        d_->writer.Double(val.toDouble());
        break;
    case Variant::STRING:
        d_->writer.String(val.toString());
        break;
    }
    return *this;
}

particle::JsonWriter& particle::JsonWriter::nullValue() {
    d_->writer.Null();
    return *this;
}
