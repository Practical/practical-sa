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

int compile(std::string path, const CompilerArguments *args) {
    // Load file into memory
    Mmap<MapMode::ReadOnly> sourceFile(path);

    AST ast;

    // Parse
    ast.parseModule(sourceFile.getSlice<const char>());

    // Semantic analysis
    ast.prepare();


    return 0;
}

} // PracticalSemanticAnalyzer
