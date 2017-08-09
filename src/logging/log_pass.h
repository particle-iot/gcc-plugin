#pragma once

#include "plugin/plugin_base.h"
#include "plugin/pass.h"
#include "plugin/tree.h"
#include "plugin/gcc.h"
#include "util/variant.h"
#include "common.h"

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
    // Logging function
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

    // Log message
    struct LogMsg {
        std::string fmtStr;
        gimple assignIdStmt;

        LogMsg(std::string fmtStr, gimple assignIdStmt) :
                fmtStr(std::move(fmtStr)),
                assignIdStmt(assignIdStmt) {
        }
    };

    typedef std::list<LogMsg> LogMsgList;

    std::map<DeclUid, LogFunc> logFuncs_;
    std::unique_ptr<MsgIndex> msgIndex_;

    void processFunc(function* fn, LogMsgList* msgList);
    void processStmt(gimple_stmt_iterator gsi, LogMsgList* msgList);
    void updateMsgIds(const LogMsgList& msgList);

    static LogFunc makeLogFunc(tree fnDecl, unsigned fmtArgIndex);
};

} // namespace particle
