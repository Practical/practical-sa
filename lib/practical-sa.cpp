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

} // PracticalSemanticAnalyzer
