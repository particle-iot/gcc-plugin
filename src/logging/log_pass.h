#pragma once

#include "plugin/pass.h"
#include "plugin/tree.h"
#include "plugin/gcc.h"
#include "util/variant.h"
#include "common.h"

#include <map>

namespace particle {

class LogPass: public Pass<gimple_opt_pass> {
public:
    explicit LogPass(gcc::context* ctx);

    // gimple_opt_pass
    virtual unsigned execute(function* fn) override;
    virtual bool gate(function* fn) override;
    virtual opt_pass* clone() override;

    void attrHandler(tree t, const std::string& name, std::vector<Variant> args);

private:
    // Description of a logging function
    struct LogFuncInfo {
        tree idFieldDecl, hasIdFieldDecl;
        unsigned fmtArgIndex, attrArgIndex;

        LogFuncInfo() :
                idFieldDecl(NULL_TREE),
                fmtArgIndex(0),
                attrArgIndex(0) {
        }
    };

    std::map<DeclUid, LogFuncInfo> logFuncs_;

    static LogFuncInfo logFuncInfo(const_tree fnDecl, unsigned fmtArgIndex);
};

} // namespace particle
