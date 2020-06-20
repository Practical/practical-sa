/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/compound_statement.h"

#include "ast/statement.h"

namespace AST {

CompoundStatement::CompoundStatement(
        const NonTerminals::CompoundStatement &parserCompound, const LookupContext &parentCtx ) :
    lookupCtx( &parentCtx ),
    statementList( parserCompound.statements )
{}

void CompoundStatement::buildAST() {
    statementList.buildAST( lookupCtx );
}

void CompoundStatement::codeGen( PracticalSemanticAnalyzer::FunctionGen *functionGen ) const {
    statementList.codeGen( lookupCtx, functionGen );
}

} // namespace AST
