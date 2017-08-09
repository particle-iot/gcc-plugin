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
