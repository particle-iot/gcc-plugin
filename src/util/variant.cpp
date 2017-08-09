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

#include "util/variant.h"

#include "util/string.h"

namespace {

using namespace particle;

class TypeVisitor: public boost::static_visitor<Variant::Type> {
public:
    Variant::Type operator()(boost::blank) const {
        return Variant::NONE;
    }

    Variant::Type operator()(bool) const {
        return Variant::BOOL;
    }

    Variant::Type operator()(int) const {
        return Variant::INT;
    }

    Variant::Type operator()(double) const {
        return Variant::DOUBLE;
    }

    Variant::Type operator()(const std::string&) const {
        return Variant::STRING;
    }
};

template<typename T>
class ValueVisitor: public boost::static_visitor<T> {
public:
    T operator()(boost::blank) const {
        return T();
    }

    T operator()(const std::string& str) const {
        return fromStr<T>(str);
    }

    template<typename ValueT>
    T operator()(ValueT val) const {
        return static_cast<T>(val);
    }
};

// Specialization for Variant::toString()
template<>
class ValueVisitor<std::string>: public boost::static_visitor<std::string> {
public:
    std::string operator()(boost::blank) const {
        return std::string();
    }

    const std::string& operator()(const std::string& str) const {
        return str;
    }

    template<typename ValueT>
    std::string operator()(ValueT val) const {
        return toStr(val);
    }
};

} // namespace

bool particle::Variant::toBool() const {
    return boost::apply_visitor(ValueVisitor<bool>(), val_);
}

int particle::Variant::toInt() const {
    return boost::apply_visitor(ValueVisitor<int>(), val_);
}

double particle::Variant::toDouble() const {
    return boost::apply_visitor(ValueVisitor<double>(), val_);
}

std::string particle::Variant::toString() const {
    return boost::apply_visitor(ValueVisitor<std::string>(), val_);
}

particle::Variant::Type particle::Variant::type() const {
    return boost::apply_visitor(TypeVisitor(), val_);
}
