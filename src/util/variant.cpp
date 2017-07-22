#include "util/variant.h"

#include "util/string.h"

namespace {

using namespace particle;
using util::Variant;

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

// Used by Variant::toBool(), Variant::toInt(), Variant::toDouble()
template<typename T>
class ValueVisitor: public boost::static_visitor<T> {
public:
    T operator()(boost::blank) const {
        return T();
    }

    T operator()(const std::string& str) const {
        return util::fromString<T>(str);
    }

    template<typename ValueT>
    T operator()(ValueT val) const {
        return static_cast<T>(val);
    }
};

// Used by Variant::toString()
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
        return util::toString(val);
    }
};

} // namespace

bool particle::util::Variant::toBool() const {
    return boost::apply_visitor(ValueVisitor<bool>(), val_);
}

int particle::util::Variant::toInt() const {
    return boost::apply_visitor(ValueVisitor<int>(), val_);
}

double particle::util::Variant::toDouble() const {
    return boost::apply_visitor(ValueVisitor<double>(), val_);
}

std::string particle::util::Variant::toString() const {
    return boost::apply_visitor(ValueVisitor<std::string>(), val_);
}

particle::util::Variant::Type particle::util::Variant::type() const {
    return boost::apply_visitor(TypeVisitor(), val_);
}
