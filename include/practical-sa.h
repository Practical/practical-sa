#ifndef LIB_PRACTICAL_SA_H
#define LIB_PRACTICAL_SA_H

#include "typed.h"
#include "slice.h"

#include <iostream>
#include <memory>
#include <string>

namespace PracticalSemanticAnalyzer {
    // Hopefully unique module ID for our typed ids
    static constexpr size_t PracticalSAModuleId = 0xc5489da402dc5a84;

    typedef Typed<unsigned long, 0, PracticalSAModuleId, __LINE__> ModuleId;
    typedef Typed<unsigned long, 0, PracticalSAModuleId, __LINE__> IdentifierId;
    typedef Typed<unsigned long, 0, PracticalSAModuleId, __LINE__> ExpressionId;

    class CompilerArguments {
    public:
    };

    struct StaticType {
        IdentifierId id;
    };

    struct VariableDeclaration {
        StaticType type;
        String name;

        VariableDeclaration(StaticType _type, String _name) : type(_type), name(_name) {}
    };

    /// Callbacks used by the semantic analyzer to allow the SA user to actually generate code
    class FunctionGen {
    public:
        virtual void functionEnter(
                IdentifierId id, String name, StaticType returnType, Slice<VariableDeclaration> arguments,
                String file, size_t line, size_t col) = 0;
        virtual void functionLeave(IdentifierId id) = 0;
    };

    class ModuleGen {
    public:
        virtual void moduleEnter(ModuleId id, String name, String file, size_t line, size_t col) = 0;
        virtual void moduleLeave(ModuleId id) = 0;

        virtual std::shared_ptr<FunctionGen> handleFunction(IdentifierId id) = 0;
    };

    std::unique_ptr<CompilerArguments> allocateArguments();

    // XXX Should path actually be a buffer?
    int compile(std::string path, const CompilerArguments *arguments, ModuleGen *codeGen);
} // End namespace PracticalSemanticAnalyzer

#endif // LIB_PRACTICAL_SA_H
