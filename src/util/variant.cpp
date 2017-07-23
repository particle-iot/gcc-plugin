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

    Variant::Type operator()(int64_t) const {
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

int64_t particle::Variant::toInt() const {
    return boost::apply_visitor(ValueVisitor<int64_t>(), val_);
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
