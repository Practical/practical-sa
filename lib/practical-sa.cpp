#include "config.h"

#include <practical-sa.h>

#include "ast.h"
#include "mmap.h"
#include "parser.h"

#include "defines.h"

namespace PracticalSemanticAnalyzer {
std::ostream &operator<<(std::ostream &out, const StaticType &type) {
    auto namedType = getTypeMeaning(type.getId());

    struct Visitor {
        String operator()(const NamedType::BuiltIn *builtIn) const {
            return builtIn->name;
        }
    };

    out << std::visit(Visitor(), namedType);

    return out;
}

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

    ASSERT( namedObject->isType() ) << "TODO implement";

    struct Visitor {
        Variant operator()(const ::BuiltInType &type) const {
            return Variant(&type);
        }

        Variant operator()(const AST::FuncDef *funcDef) const {
            ABORT() << "TODO impelement";
        }
    };

    return std::visit(Visitor(), namedObject->definition);
}

} // PracticalSemanticAnalyzer
