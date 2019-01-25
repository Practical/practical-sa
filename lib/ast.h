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

class AST {
    LookupContext globalCtx;
    std::unordered_map<String, std::unique_ptr<NonTerminals::Module>> parsedModules;
    std::unordered_map<String, std::unique_ptr<::AST::Module>> modulesAST;

public:

    AST() : globalCtx(nullptr) {
    }

    void prepare();

    void parseModule(String moduleSource);

    void codeGen(PracticalSemanticAnalyzer::ModuleGen *codeGen);
};

} // namespace AST

#endif // AST_H
