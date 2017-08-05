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
