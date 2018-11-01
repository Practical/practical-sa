#ifndef AST_H
#define AST_H

#include "lookup_context.h"
#include "parser.h"

#include <memory>

struct AST {
    LookupContext globalCtx;
    std::unordered_map<String, std::unique_ptr<NonTerminals::Module>> modules;

    void prepare();

    void parseModule(String moduleSource);
};

#endif // AST_H
