#include "ast.h"

void AST::prepare()
{
    // Register the built-in types
    globalCtx.registerBuiltInType("Void", BuiltInType::Type::Void, 0);

    globalCtx.registerBuiltInType("S8", BuiltInType::Type::SignedInt, 1);
    globalCtx.registerBuiltInType("S16", BuiltInType::Type::SignedInt, 2);
    globalCtx.registerBuiltInType("S32", BuiltInType::Type::SignedInt, 4);
    globalCtx.registerBuiltInType("S64", BuiltInType::Type::SignedInt, 8);
    globalCtx.registerBuiltInType("S128", BuiltInType::Type::SignedInt, 16);

    globalCtx.registerBuiltInType("U8", BuiltInType::Type::UnsignedInt, 1);
    globalCtx.registerBuiltInType("U16", BuiltInType::Type::UnsignedInt, 2);
    globalCtx.registerBuiltInType("U32", BuiltInType::Type::UnsignedInt, 4);
    globalCtx.registerBuiltInType("U64", BuiltInType::Type::UnsignedInt, 8);
    globalCtx.registerBuiltInType("U128", BuiltInType::Type::UnsignedInt, 16);
}

void AST::parseModule(String moduleSource) {
    auto module = safenew<NonTerminals::Module>(&globalCtx);
    module->parse(moduleSource);

    module->symbolsPass1(&globalCtx);
    module->symbolsPass2(&globalCtx);

    modules.emplace(toSlice("Dummy module"), std::move(module));
}

void AST::codeGen(PracticalSemanticAnalyzer::CodeGen *codeGen) {
    // XXX We should only code-gen some of the modules?
    for(auto &module: modules) {
        module.second->codeGen(codeGen);
    }
}
