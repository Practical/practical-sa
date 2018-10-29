#ifndef LIB_PRACTICAL_SA_H
#define LIB_PRACTICAL_SA_H

#include <memory>
#include <string>

namespace PracticalSemanticAnalyzer {
    class CompilerArguments {
    public:
    };

    std::unique_ptr<CompilerArguments> allocateArguments();

    // XXX Should path actually be a buffer?
    int compile(std::string path, const CompilerArguments *arguments);

    /// Callbacks used by the semantic analyzer to allow the SA user to actually generate code
    struct CodeGenHooks {
    };
} // End namespace PracticalSemanticAnalyzer

#endif // LIB_PRACTICAL_SA_H
