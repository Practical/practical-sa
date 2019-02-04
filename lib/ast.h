#ifndef AST_H
#define AST_H

#include "ast_nodes.h"
#include "lookup_context.h"
#include "parser.h"
#include "practical-sa.h"

#include <memory>

extern PracticalSemanticAnalyzer::ExpressionId::Allocator<> expressionIdAllocator;
extern PracticalSemanticAnalyzer::ExpressionId voidExpressionId;
extern PracticalSemanticAnalyzer::ModuleId::Allocator<> moduleIdAllocator;

namespace AST {

class semantic_error : public compile_error {
public:
    semantic_error(const char *msg, size_t line, size_t col) : compile_error(msg, line, col) {
    }
};

class AST {
    static LookupContext globalCtx;
    std::unordered_map<String, std::unique_ptr<NonTerminals::Module>> parsedModules;
    std::unordered_map<String, std::unique_ptr<::AST::Module>> modulesAST;

public:

    AST() {
    }

    static const LookupContext &getGlobalCtx() {
        return globalCtx;
    }
    static StaticType deductLiteralRange(LongEnoughInt value);

    void prepare();

    void parseModule(String moduleSource);

    void codeGen(PracticalSemanticAnalyzer::ModuleGen *codeGen);
};

bool implicitCastAllowed(const StaticType &sourceType, const StaticType &destType, const LookupContext &ctx);

} // namespace AST

#endif // AST_H
