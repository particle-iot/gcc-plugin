#pragma once

#include "common.h"

#include <boost/variant.hpp>

namespace particle {

namespace util {

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
    Variant(double val);
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

} // namespace util

} // namespace particle

inline particle::util::Variant::Variant() {
}

inline particle::util::Variant::Variant(bool val) :
        val_(val) {
}

inline particle::util::Variant::Variant(int val) :
        val_(val) {
}

inline particle::util::Variant::Variant(double val) :
        val_(val) {
}

inline particle::util::Variant::Variant(std::string str) :
        val_(std::move(str)) {
}

inline bool particle::util::Variant::isNone() const {
    return (type() == NONE);
}

inline bool particle::util::Variant::isBool() const {
    return (type() == BOOL);
}

inline bool particle::util::Variant::isInt() const {
    return (type() == INT);
}

inline bool particle::util::Variant::isDouble() const {
    return (type() == DOUBLE);
}

inline bool particle::util::Variant::isString() const {
    return (type() == STRING);
}
