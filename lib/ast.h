#ifndef AST_H
#define AST_H

#include "lookup_context.h"
#include "parser.h"
#include "practical-sa.h"

#include <memory>

extern PracticalSemanticAnalyzer::ExpressionId::Allocator<> expressionIdAllocator;
extern PracticalSemanticAnalyzer::ExpressionId voidExpressionId;

struct AST {
    LookupContext globalCtx;
    std::unordered_map<String, std::unique_ptr<NonTerminals::Module>> modules;

    AST() : globalCtx(nullptr) {
    }

    void prepare();

    void parseModule(String moduleSource);

    void codeGen(PracticalSemanticAnalyzer::ModuleGen *codeGen);
};

#endif // AST_H
