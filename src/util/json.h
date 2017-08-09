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

#pragma once

#include "util/variant.h"
#include "common.h"

#include <iostream>

namespace particle {

// SAX-alike parser for JSON data
class JsonReader {
public:
    class Handler;

    JsonReader(std::istream* strm, Handler* handler);
    ~JsonReader();

    void parse();

private:
    struct Data;

    std::unique_ptr<Data> d_;
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

class JsonWriter {
public:
    explicit JsonWriter(std::ostream* strm);
    ~JsonWriter();

    JsonWriter& beginObject();
    JsonWriter& endObject();
    JsonWriter& beginArray();
    JsonWriter& endArray();
    JsonWriter& name(const std::string& name);
    JsonWriter& value(const Variant& val);
    JsonWriter& nullValue(); // Equivalent to value(Variant())

private:
    struct Data;

    std::unique_ptr<Data> d_;
};

} // namespace particle

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
