#include "ast.h"

void AST::prepare()
{
    // Register the built-in types
    globalCtx.registerBuiltInType("S8");
    globalCtx.registerBuiltInType("S16");
    globalCtx.registerBuiltInType("S32");
    globalCtx.registerBuiltInType("S64");
    globalCtx.registerBuiltInType("S128");

    globalCtx.registerBuiltInType("U8");
    globalCtx.registerBuiltInType("U16");
    globalCtx.registerBuiltInType("U32");
    globalCtx.registerBuiltInType("U64");
    globalCtx.registerBuiltInType("U128");
}

void AST::parseModule(String moduleSource) {
    auto module = safenew<NonTerminals::Module>(&globalCtx);
    module->parse(moduleSource);

    module->symbolsPass1(&globalCtx);

    modules.emplace(toSlice("Dummy module"), std::move(module));
}

void AST::codeGen(PracticalSemanticAnalyzer::CodeGen *codeGen) {
    // XXX We should only code-gen some of the modules?
    for(auto &module: modules) {
        module.second->codeGen(codeGen);
    }
}
