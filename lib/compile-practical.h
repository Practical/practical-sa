#ifndef LIB_COMPILE_PRACTICAL_H
#define LIB_COMPILE_PRACTICAL_H

namespace PracticalCompiler {
    class CompilerArguments;

    std::unique_ptr<CompilerArguments> allocateArguments;

    // XXX Should path actually be a buffer?
    int compile(std::string path, std::unique_ptr<CompilerArguments> arguments);

} // End namespace PracticalCompiler

#endif // LIB_COMPILE_PRACTICAL_H
