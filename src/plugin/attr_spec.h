#pragma once

#include "plugin/location.h"
#include "plugin/gcc.h"
#include "util/variant.h"
#include "common.h"

#include <vector>

namespace particle {

class AttrSpec {
public:
    // Handler function
    typedef std::function<void(tree, const std::vector<Variant>&)> Handler;

    AttrSpec(std::string name = std::string());

    AttrSpec& name(std::string name);
    const std::string& name() const;

    AttrSpec& minArgCount(int n);
    int minArgCount() const;

    // Set to -1 if the attribute takes unspecified number of arguments (default)
    AttrSpec& maxArgCount(int n);
    int maxArgCount() const;

    // Sets fixed number of arguments
    AttrSpec& argCount(int n);

    AttrSpec& handler(Handler handler);
    const Handler& handler() const;

private:
    Handler handler_;
    std::string name_;
    int minArgCount_, maxArgCount_;
};

} // namespace particle

inline particle::AttrSpec::AttrSpec(std::string name) :
        name_(std::move(name)),
        minArgCount_(0),
        maxArgCount_(-1) {
}

inline particle::AttrSpec& particle::AttrSpec::name(std::string name) {
    name_ = std::move(name);
    return *this;
}

inline const std::string& particle::AttrSpec::name() const {
    return name_;
}

inline particle::AttrSpec& particle::AttrSpec::minArgCount(int n) {
    minArgCount_ = n;
    return *this;
}

inline int particle::AttrSpec::minArgCount() const {
    return minArgCount_;
}

inline particle::AttrSpec& particle::AttrSpec::maxArgCount(int n) {
    maxArgCount_ = n;
    return *this;
}

inline int particle::AttrSpec::maxArgCount() const {
    return maxArgCount_;
}

inline particle::AttrSpec& particle::AttrSpec::argCount(int n) {
    minArgCount_ = n;
    maxArgCount_ = n;
    return *this;
}

inline particle::AttrSpec& particle::AttrSpec::handler(Handler handler) {
    handler_ = std::move(handler);
    return *this;
}

inline const particle::AttrSpec::Handler& particle::AttrSpec::handler() const {
    return handler_;
}
