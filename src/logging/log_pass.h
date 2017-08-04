#pragma once

#include "plugin/plugin_base.h"
#include "plugin/pass.h"
#include "plugin/tree.h"
#include "plugin/gcc.h"
#include "util/variant.h"
#include "common.h"

#include <boost/filesystem/path.hpp>

#include <map>

namespace particle {

class MsgIndex;

class LogPass: public Pass<simple_ipa_opt_pass> {
public:
    LogPass(gcc::context* ctx, const PluginArgs& args);
    virtual ~LogPass();

    // opt_pass
    virtual unsigned execute(function* fn) override;
    virtual bool gate(function* fn) override;
    virtual opt_pass* clone() override;

    // Called by the Plugin instance
    void attrHandler(tree t, const std::string& name, std::vector<Variant> args);

private:
    // Description of a logging function
    struct LogFunc {
        tree idFieldDecl, hasIdFieldDecl;
        unsigned fmtArgIndex, attrArgIndex;

        LogFunc() :
                idFieldDecl(NULL_TREE),
                hasIdFieldDecl(NULL_TREE),
                fmtArgIndex(0),
                attrArgIndex(0) {
        }
    };

    std::map<DeclUid, LogFunc> logFuncs_;
    std::unique_ptr<MsgIndex> msgIndex_;
    boost::filesystem::path srcIndexFile_, destIndexFile_;

    void processFunc(function* fn);
    void processGimpleStmt(gimple_stmt_iterator gsi);

    MsgIndex* msgIndex();

    static LogFunc makeLogFunc(tree fnDecl, unsigned fmtArgIndex);
};

} // namespace particle
