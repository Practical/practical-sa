#ifndef AST_H
#define AST_H

#include "lookup_context.h"
#include "parser.h"
#include "practical-sa.h"

#include <memory>

struct AST {
    LookupContext globalCtx;
    std::unordered_map<String, std::unique_ptr<NonTerminals::Module>> modules;

    AST() : globalCtx(nullptr) {
    }

    void prepare();

    void parseModule(String moduleSource);

    void codeGen(PracticalSemanticAnalyzer::CodeGen *codeGen);
};

#endif // AST_H
