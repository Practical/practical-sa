#include "config.h"

#include <practical-sa.h>

#include "mmap.h"
#include "parser.h"
#include "semantic_analyzer.h"

#include "defines.h"

namespace PracticalSemanticAnalyzer {
std::unique_ptr<CompilerArguments> allocateArguments() {
    return safenew<CompilerArguments>();
}

int compile(std::string path, const CompilerArguments *args) {
    // Load file into memory
    Mmap<MapMode::ReadOnly> sourceFile(path);

    // Tokenize
    auto tokens = Tokenizer::tokenize(sourceFile.getSlice<const char>());

    // Parse
    Parser::NonTerminals::Module moduleAst;
    moduleAst.parse( tokens );

    // Semantic analysis
    SemanticAnalyzer::Context moduleCtx(moduleAst);

    return 0;
}

} // PracticalSemanticAnalyzer
