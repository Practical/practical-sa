#include "config.h"

#include <practical-sa.h>

#include "ast.h"
#include "mmap.h"
#include "parser.h"

#include "defines.h"

namespace PracticalSemanticAnalyzer {
std::unique_ptr<CompilerArguments> allocateArguments() {
    return safenew<CompilerArguments>();
}

int compile(std::string path, const CompilerArguments *arguments, ModuleGen *codeGen) {
    // Load file into memory
    Mmap<MapMode::ReadOnly> sourceFile(path);

    AST::AST ast;

    // Parse + symbols lookup
    ast.prepare();
    ast.parseModule(sourceFile.getSlice<const char>());

    // And that other thing
    ast.codeGen(codeGen);

    return 0;
}

std::variant<const NamedType::BuiltIn *> getTypeMeaning(IdentifierId id) {
    using Variant = std::variant<const NamedType::BuiltIn *>;

    auto namedObject = LookupContext::lookupIdentifier(id);

    assert( namedObject->isType() ); // TODO implement

    struct Visitor {
        Variant operator()(const ::BuiltInType &type) const {
            return Variant(&type);
        }

        Variant operator()(const AST::FuncDef *funcDef) const {
            abort(); // TODO impelement
        }
    };

    return std::visit(Visitor(), namedObject->definition);
}

} // PracticalSemanticAnalyzer
