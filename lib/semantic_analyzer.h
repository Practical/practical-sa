#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "practical-sa.h"

#include "nocopy.h"
#include "parser.h"
#include "slice.h"

#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

namespace SemanticAnalyzer {

// A Context is anything that may contain further symbolic definitions. This may be a module, a struct, a function or even an
// anonymous block of code.
class Context : private NoCopy {
    struct NamedObject {
        enum class Type { FunctionDef };
        std::variant< Parser::NonTerminals::FuncDef * > definition;
    };

    // My place in the world
    Context *parent = nullptr;
    std::vector< std::unique_ptr<Context> > children;

    enum class Type { Module, Function } type;

    std::unordered_map<String, NamedObject> symbols;

public:
    Context(Parser::NonTerminals::Module &module);

    void generateCode(const PracticalSemanticAnalyzer::CodeGenHooks *hooks);

private:
    // Private constructors
    Context(const Parser::NonTerminals::FuncDef &function);
};


}

#endif // SEMANTIC_ANALYZER_H
