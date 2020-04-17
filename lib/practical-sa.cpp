/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * This file is file is copyright (C) 2018-2019 by its authors.
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "config.h"

#include <practical-sa.h>

#include "ast/ast.h"
#include "ast/static_type.h"
#include "mmap.h"
#include "parser.h"

#include "defines.h"

DEF_TYPED_NS( PracticalSemanticAnalyzer, ModuleId );
DEF_TYPED_NS( PracticalSemanticAnalyzer, ExpressionId );
DEF_TYPED_NS( PracticalSemanticAnalyzer, JumpPointId );

namespace PracticalSemanticAnalyzer {

std::ostream &operator<<(std::ostream &out, StaticType::CPtr type) {
    struct Visitor {
        std::ostream &out;

        void operator()( const StaticType::Scalar *scalar ) {
            out<<scalar->getName();
        }

        void operator()( const StaticType::Function *function ) {
            out<<"Function of some type";
            ABORT()<<"TODO implement";
        }
    };

    std::visit( Visitor{ .out = out }, type->getType() );

    return out;
}

std::unique_ptr<CompilerArguments> allocateArguments() {
    return safenew<CompilerArguments>();
}

void prepare( BuiltinContextGen *ctxGen ) {
    AST::AST::prepare(ctxGen);
}

int compile(std::string path, const CompilerArguments *arguments, ModuleGen *codeGen) {
    // Load file into memory
    Mmap<MapMode::ReadOnly> sourceFile(path);

    AST::AST ast;

    // Parse + symbols lookup
    ASSERT( AST::AST::prepared() )<<"compile called without calling prepare first";
    auto tokenizedModule = Tokenizer::Tokenizer::tokenize( sourceFile.getSlice<const char>() );
    NonTerminals::Module module;
    module.parse( tokenizedModule );

    // And that other thing
    ast.codeGen( module, codeGen );

    return 0;
}

} // PracticalSemanticAnalyzer
