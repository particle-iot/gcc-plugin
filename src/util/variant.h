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

#include "common.h"

#include <boost/variant.hpp>

namespace particle {

class Variant {
public:
    enum Type {
        NONE,
        BOOL,
        INT,
        DOUBLE,
        STRING
    };

    Variant();
    Variant(bool val);
    Variant(int val);
    Variant(unsigned val);
    Variant(double val);
    Variant(const char* str);
    Variant(std::string str);

    bool toBool() const;
    int toInt() const;
    double toDouble() const;
    std::string toString() const;

    Type type() const;

    bool isNone() const;
    bool isBool() const;
    bool isInt() const;
    bool isDouble() const;
    bool isString() const;

private:
    typedef boost::variant<boost::blank, bool, int, double, std::string> ValueType;
    ValueType val_;
};

} // namespace particle

inline particle::Variant::Variant() {
}

inline particle::Variant::Variant(bool val) :
        val_(val) {
}

inline particle::Variant::Variant(int val) :
        val_(val) {
}

inline particle::Variant::Variant(unsigned val) :
        val_((int)val) {
}

inline particle::Variant::Variant(double val) :
        val_(val) {
}

inline particle::Variant::Variant(const char* str) :
        val_(std::string(str)) {
}

inline particle::Variant::Variant(std::string str) :
        val_(std::move(str)) {
}

inline bool particle::Variant::isNone() const {
    return (type() == NONE);
}

inline bool particle::Variant::isBool() const {
    return (type() == BOOL);
}

inline bool particle::Variant::isInt() const {
    return (type() == INT);
}

inline bool particle::Variant::isDouble() const {
    return (type() == DOUBLE);
}

inline bool particle::Variant::isString() const {
    return (type() == STRING);
}
