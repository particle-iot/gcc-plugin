#pragma once

#include "plugin/plugin_base.h"
#include "plugin/pass.h"
#include "plugin/tree.h"
#include "plugin/gcc.h"
#include "util/variant.h"
#include "common.h"

#include <unordered_map>
#include <map>
#include <list>

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

    // Description of a log message
    struct LogMsg {
        std::list<gimple> assignIdStmts;
    };

    typedef std::unordered_map<std::string, LogMsg> LogMsgMap;

    std::map<DeclUid, LogFunc> logFuncs_;
    std::unique_ptr<MsgIndex> msgIndex_;

    void processFunc(function* fn, LogMsgMap* msgMap);
    void processStmt(gimple_stmt_iterator gsi, LogMsgMap* msgMap);
    void updateMsgIds(const LogMsgMap& msgMap);

    static LogFunc makeLogFunc(tree fnDecl, unsigned fmtArgIndex);
};

} // namespace particle
