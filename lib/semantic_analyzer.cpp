#include "semantic_analyzer.h"

#include "parser.h"
#include "practical-sa.h"

namespace SemanticAnalyzer {

void Context::generateCode(const PracticalSemanticAnalyzer::CodeGenHooks *hooks) {
}

Context::Context(Parser::NonTerminals::Module &module) : type(Type::Module) {
    for( auto &func : module.functionDefinitions ) {
        Context::NamedObject def;
        def.definition = &func;

        auto previousDefinition = symbols.find(func.name());
        if( previousDefinition == symbols.end() ) {
            symbols[func.name()] = def;
            // safenew doesn't work here, because we're invoking a private constructor. Thankfully, it is also not needed, as
            // emplace will invoke the unique_ptr constructor that accepts a naked pointer.
            children.emplace_back( new Context(func) );
        } else {
            // XXX Overloads not yet implemented
            assert(false);
        }
    }
}

Context::Context(const Parser::NonTerminals::FuncDef &function) : type(Type::Function) {
    // XXX Definitions inside function not yet handled
}

}
