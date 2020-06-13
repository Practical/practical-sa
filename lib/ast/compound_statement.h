/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_COMPOUND_STATEMENT_H
#define AST_COMPOUND_STATEMENT_H

#include "lookup_context.h"
#include "parser.h"

namespace AST {

class CompoundStatement {
    const NonTerminals::CompoundStatement &parserCompound;
    LookupContext ctx;

public:
    explicit CompoundStatement( const NonTerminals::CompoundStatement &parserCompound, const LookupContext &parentCtx );
    void codeGen( PracticalSemanticAnalyzer::FunctionGen *functionGen );
};

} // namespace AST

#endif // AST_COMPOUND_STATEMENT_H
