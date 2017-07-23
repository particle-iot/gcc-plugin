#pragma once

#include "plugin/gcc.h"
#include "common.h"

namespace particle {

class PassInfo {
public:
    explicit PassInfo(opt_pass* pass = nullptr);

    PassInfo& pass(opt_pass* pass);
    opt_pass* pass() const;

    PassInfo& refPassName(std::string name);
    const std::string& refPassName() const;

    // Set to 0 to run this pass for every invocation of the reference pass (default)
    PassInfo& refPassInstanceNum(int n);
    int refPassInstanceNum() const;

    // PASS_POS_INSERT_AFTER: insert after the reference pass
    // PASS_POS_INSERT_BEFORE: insert before the reference pass
    // PASS_POS_REPLACE: replace the reference pass
    PassInfo& pos(pass_positioning_ops pos);
    pass_positioning_ops pos() const;

    // Convenience methods
    PassInfo& runAfter(std::string passName);
    PassInfo& runBefore(std::string passName);
    PassInfo& runInsteadOf(std::string passName);

private:
    opt_pass* pass_;
    std::string refPass_;
    pass_positioning_ops pos_;
    int refPassNum_;
};

} // namespace particle

inline particle::PassInfo::PassInfo(opt_pass* pass) :
        pass_(pass),
        pos_(PASS_POS_INSERT_AFTER),
        refPassNum_(0) {
}

inline particle::PassInfo& particle::PassInfo::pass(opt_pass* pass) {
    pass_ = pass;
    return *this;
}

inline opt_pass* particle::PassInfo::pass() const {
    return pass_;
}

inline particle::PassInfo& particle::PassInfo::refPassName(std::string name) {
    refPass_ = std::move(name);
    return *this;
}

inline const std::string& particle::PassInfo::refPassName() const {
    return refPass_;
}

inline particle::PassInfo& particle::PassInfo::refPassInstanceNum(int n) {
    refPassNum_ = n;
    return *this;
}

inline int particle::PassInfo::refPassInstanceNum() const {
    return refPassNum_;
}

inline particle::PassInfo& particle::PassInfo::pos(pass_positioning_ops pos) {
    pos_ = pos;
    return *this;
}

inline pass_positioning_ops particle::PassInfo::pos() const {
    return pos_;
}

inline particle::PassInfo& particle::PassInfo::runAfter(std::string passName) {
    return refPassName(std::move(passName)).pos(PASS_POS_INSERT_AFTER);
}

inline particle::PassInfo& particle::PassInfo::runBefore(std::string passName) {
    return refPassName(std::move(passName)).pos(PASS_POS_INSERT_BEFORE);
}

inline particle::PassInfo& particle::PassInfo::runInsteadOf(std::string passName) {
    return refPassName(std::move(passName)).pos(PASS_POS_REPLACE);
}
