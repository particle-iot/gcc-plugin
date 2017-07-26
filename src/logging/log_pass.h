#pragma once

#include "plugin/pass_info.h"
#include "plugin/location.h"
#include "plugin/gcc.h"
#include "util/variant.h"
#include "common.h"

#include <map>

namespace particle {

class LogPass: public gimple_opt_pass {
public:
    explicit LogPass(gcc::context* ctx);

    virtual unsigned execute(function* fn) override;
    virtual bool gate(function* fn) override;
    virtual opt_pass* clone() override;

    void attrHandler(tree t, const std::string& name, std::vector<Variant> args);

    PassInfo passInfo();

private:
    struct LogFuncInfo {
        unsigned fmtArgIndex, attrArgIndex;
    };

    std::map<Location, LogFuncInfo> logFuncs_;
};

} // namespace particle

inline particle::PassInfo particle::LogPass::passInfo() {
    return PassInfo(this).runAfter("cfg");
}
