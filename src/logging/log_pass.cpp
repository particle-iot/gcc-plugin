#include "logging/log_pass.h"

#include "logging/msg_index.h"
#include "plugin/gimple.h"
#include "debug.h"

// Uncomment to enable more debugging output
// #define DEBUG_LOG_PASS(...) DEBUG(__VA_ARGS__)

#ifndef DEBUG_LOG_PASS
#define DEBUG_LOG_PASS(...)
#endif

namespace {

using namespace particle;

const pass_data LOG_PASS_DATA = {
    SIMPLE_IPA_PASS, // type
    "particle_log_pass", // name
    OPTGROUP_NONE, // optinfo_flags
    TV_NONE, // tv_id
    0, // properties_required
    0, // properties_provided
    0, // properties_destroyed
    0, // todo_flags_start
    0 // todo_flags_finish
};

const std::string LOG_ATTR_STRUCT = "LogAttributes";
const std::string LOG_ATTR_ID_FIELD = "id";
const std::string LOG_ATTR_HAS_ID_FIELD = "has_id";

// Returns reference (a COMPONENT_REF node) to a structure's field, which may be nested in a number of
// anonymous struct/union fields. GCC already provides build_component_ref() function that does the
// job, but unfortunately it's only available for C code at runtime
tree buildComponentRef(tree ref, tree field) {
    assert(ref != NULL_TREE && field != NULL_TREE);
    tree next = TREE_CHAIN(field);
    field = TREE_VALUE(field);
    ref = build3_loc(UNKNOWN_LOCATION, COMPONENT_REF, TREE_TYPE(field), ref, field, NULL_TREE);
    if (next != NULL_TREE) {
        ref = buildComponentRef(ref, next);
    }
    return ref;
}

// Returns field declaration for given structure type and field name
tree findFieldDecl(tree structType, const std::string& fieldName) {
    for (tree field = TYPE_FIELDS(structType); field != NULL_TREE; field = TREE_CHAIN(field)) {
        tree name = DECL_NAME(field);
        if (name == NULL_TREE) {
            tree type = TREE_TYPE(field);
            if (RECORD_OR_UNION_TYPE_P(type)) { // Anonymous struct/union
                tree list = findFieldDecl(type, fieldName);
                if (list != NULL_TREE) {
                    return tree_cons(NULL_TREE, field, list);
                }
            }
        } else if (IDENTIFIER_POINTER(name) == fieldName) {
            return tree_cons(NULL_TREE, field, NULL_TREE);
        }
    }
    return NULL_TREE;
}

} // namespace

particle::LogPass::LogPass(gcc::context* ctx, const PluginArgs& args) :
        Pass<BaseType>(LOG_PASS_DATA, ctx) {
    // Destination index file
    std::string destIndexFile;
    auto it = args.find("msg-index");
    if (it != args.end()) {
        destIndexFile = it->second.toString();
    }
    if (!destIndexFile.empty()) {
        // Predefined index file (optional)
        std::string predefIndexFile;
        it = args.find("msg-index-predef");
        if (it != args.end()) {
            predefIndexFile = it->second.toString();
        }
        msgIndex_.reset(new MsgIndex(destIndexFile, predefIndexFile));
    }
}

particle::LogPass::~LogPass() {
}

unsigned particle::LogPass::execute(function*) {
    try {
        // Collect all log messages
        LogMsgMap msgMap;
        cgraph_node *node = nullptr;
        FOR_EACH_DEFINED_FUNCTION(node) {
            function* const fn = node->get_fun();
            assert(fn);
            processFunc(fn, &msgMap);
        }
        // Update message IDs
        updateMsgIds(msgMap);
    } catch (const PassError& e) {
        error(e.location(), e.message());
    } catch (const std::exception& e) {
        // Include plugin name for better readability
        error("%s: %s", PluginBase::instance()->pluginName(), e.what());
    }
    return 0; // No additional TODOs
}

bool particle::LogPass::gate(function*) {
    // Run this pass only if there are logging functions declared in current translation unit
    return (msgIndex_ && !logFuncs_.empty());
}

opt_pass* particle::LogPass::clone() {
    return this; // FIXME?
}

void particle::LogPass::attrHandler(tree t, const std::string& name, std::vector<Variant> args) {
    if (name != "log_function") {
        throw Error("Invalid attribute argument: \"%s\"", name);
    }
    if (TREE_CODE(t) != FUNCTION_DECL) {
        throw Error("This attribute can be applied only to function declarations");
    }
    DEBUG_LOG_PASS("%s: Logging function: %s()", location(t).str(), declName(t));
    if (args.size() != 1) {
        throw Error("Invalid number of attribute arguments");
    }
    const int fmtArgIndex = args.at(0).toInt() - 1; // Convert to 0-based index
    if (fmtArgIndex < 0) {
        throw Error("Invalid index of the format string argument");
    }
    logFuncs_[DECL_UID(t)] = makeLogFunc(t, fmtArgIndex);
}

void particle::LogPass::processFunc(function* fn, LogMsgMap* msgMap) {
    // Iterate over all GIMPLE statements in all basic blocks
    basic_block bb = { 0 };
    FOR_ALL_BB_FN(bb, fn) {
        for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
            processStmt(gsi, msgMap);
        }
    }
}

void particle::LogPass::processStmt(gimple_stmt_iterator gsi, LogMsgMap* msgMap) {
    gimple stmt = gsi_stmt(gsi);
    if (!is_gimple_call(stmt)) {
        return; // Not a function call
    }
    // Get declaration of the called function
    tree fnDecl = gimple_call_fndecl(stmt);
    const auto logFuncIt = logFuncs_.find(DECL_UID(fnDecl));
    if (logFuncIt == logFuncs_.end()) {
        return; // Not a logging function
    }
    const LogFunc& logFunc = logFuncIt->second;
    const unsigned argCount = gimple_call_num_args(stmt);
    if (logFunc.fmtArgIndex >= argCount || logFunc.attrArgIndex >= argCount) {
        warning(location(stmt), "Unexpected number of arguments");
        return;
    }
    // Get format string argment
    tree t = gimple_call_arg(stmt, logFunc.fmtArgIndex);
    if (TREE_CODE(t) != ADDR_EXPR || TREE_OPERAND_LENGTH(t) == 0) {
        return;
    }
    t = TREE_OPERAND(t, 0);
    if (TREE_CODE(t) != STRING_CST) {
        return; // Not a string constant
    }
    const std::string fmtStr = constStrVal(t); // Format string
    // Get attributes argument
    t = gimple_call_arg(stmt, logFunc.attrArgIndex);
    if (TREE_CODE(t) != ADDR_EXPR || TREE_OPERAND_LENGTH(t) == 0) {
        return;
    }
    // Get declaration of the attributes variable
    t = TREE_OPERAND(t, 0);
    if (TREE_CODE(t) != VAR_DECL) {
        return;
    }
    DEBUG_LOG_PASS("%s: Log message: \"%s\"", location(stmt).str(), fmtStr);
    // Set `LogAttributes::id` field
    tree lhs = buildComponentRef(t, logFunc.idFieldDecl);
    tree rhs = build_int_cst(integer_type_node, 0); // Placeholder for a message ID value
    gimple assignId = gimple_build_assign(lhs, rhs);
    gsi_insert_before(&gsi, assignId, GSI_SAME_STMT);
    // Set `LogAttributes::has_id` field
    lhs = buildComponentRef(t, logFunc.hasIdFieldDecl);
    rhs = build_int_cst(integer_type_node, 1);
    gimple assignHasId = gimple_build_assign(lhs, rhs);
    gsi_insert_before(&gsi, assignHasId, GSI_SAME_STMT);
    // Update message map
    assert(msgMap);
    auto msgIt = msgMap->insert(std::make_pair(fmtStr, LogMsg())).first;
    msgIt->second.assignIdStmts.push_back(assignId);
}

void particle::LogPass::updateMsgIds(const LogMsgMap& msgMap) {
    assert(msgIndex_);
    msgIndex_->process(msgMap,
        [](const LogMsgMap::const_iterator& it) {
            return it->first; // Format string
        },
        [](const LogMsgMap::const_iterator& it, MsgId id) {
            // Update log statements with a correct message ID value
            tree rhs = build_int_cst(integer_type_node, id);
            for (gimple stmt: it->second.assignIdStmts) {
                gimple_assign_set_rhs1(stmt, rhs);
            }
        }
    );
}

particle::LogPass::LogFunc particle::LogPass::makeLogFunc(tree fnDecl, unsigned fmtArgIndex) {
    const Location loc = location(fnDecl);
    tree fmtType = NULL_TREE, attrType = NULL_TREE;
    unsigned attrArgIndex = 0, argCount = 0;
    for (tree arg = TYPE_ARG_TYPES(TREE_TYPE(fnDecl)); arg != NULL_TREE; arg = TREE_CHAIN(arg), ++argCount) {
        tree t = TREE_VALUE(arg);
        if (fmtType == NULL_TREE && argCount == fmtArgIndex) {
            // Format string argument
            fmtType = t;
            continue;
        }
        if (TREE_CODE(t) != POINTER_TYPE) {
            continue; // Not a pointer type
        }
        t = TREE_TYPE(t);
        if (TREE_CODE(t) != RECORD_TYPE) {
            continue; // Target type is not a struct
        }
        if (typeName(t) != LOG_ATTR_STRUCT) {
            continue;
        }
        if (attrType == NULL_TREE) {
            // Attributes argument
            attrType = t;
            attrArgIndex = argCount;
        } else {
            throw PassError(loc, "Logging function takes multiple `%s*` arguments", LOG_ATTR_STRUCT);
        }
    }
    if (fmtType == NULL_TREE) {
        throw PassError(loc, "Invalid index of the format string argument");
    }
    if (!isConstCharPtr(fmtType)) {
        throw PassError(loc, "Format string argument is not a `const char*` string");
    }
    if (attrType == NULL_TREE) {
        throw PassError(loc, "Logging function is expected to take `%s*` argument", LOG_ATTR_STRUCT);
    }
    if (!COMPLETE_TYPE_P(attrType)) {
        throw PassError(loc, "`%s` is an incomplete type", LOG_ATTR_STRUCT);
    }
    LogFunc logFunc;
    logFunc.fmtArgIndex = fmtArgIndex;
    logFunc.attrArgIndex = attrArgIndex;
    logFunc.idFieldDecl = findFieldDecl(attrType, LOG_ATTR_ID_FIELD);
    if (logFunc.idFieldDecl == NULL_TREE) {
        throw PassError(loc, "`%s` is missing `%s` field", LOG_ATTR_STRUCT, LOG_ATTR_ID_FIELD);
    }
    logFunc.hasIdFieldDecl = findFieldDecl(attrType, LOG_ATTR_HAS_ID_FIELD);
    if (logFunc.hasIdFieldDecl == NULL_TREE) {
        throw PassError(loc, "`%s` is missing `%s` field", LOG_ATTR_STRUCT, LOG_ATTR_HAS_ID_FIELD);
    }
    return logFunc;
}
