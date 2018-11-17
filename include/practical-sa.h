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

    enum class VisitMode { Enter, Exit };

    static inline std::ostream &operator<<(std::ostream &out, VisitMode mode) {
        switch(mode) {
        case VisitMode::Enter:
            out << "Enter";
            break;
        case VisitMode::Exit:
            out << "Exit";
            break;
        }

        return out;
    }

    class CompilerArguments {
    public:
    };

    /// Callbacks used by the semantic analyzer to allow the SA user to actually generate code
    class CodeGen {
    public:
        virtual void module(VisitMode mode, ModuleId id, String name, String file, size_t line, size_t col) = 0;
    };

    std::unique_ptr<CompilerArguments> allocateArguments();

    // XXX Should path actually be a buffer?
    int compile(std::string path, const CompilerArguments *arguments, CodeGen *codeGen);
} // End namespace PracticalSemanticAnalyzer

#endif // LIB_PRACTICAL_SA_H
