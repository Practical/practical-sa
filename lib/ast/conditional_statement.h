/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_CONDITIONAL_STATEMENT_H
#define AST_CONDITIONAL_STATEMENT_H

#include "expression.h"
#include "lookup_context.h"
#include "parser.h"

namespace AST {

class Statement;

class ConditionalStatement {
    Expression condition;
    std::unique_ptr<Statement> ifClause, elseClause;

public:
    explicit ConditionalStatement( const NonTerminals::Statement::ConditionalStatement &parserCondition );

    void buildAST( LookupContext &lookupCtx );
    void codeGen( const LookupContext &lookupCtx, PracticalSemanticAnalyzer::FunctionGen *functionGen ) const;
};

} // namespace AST::ExpressionImpl

#endif // AST_CONDITIONAL_STATEMENT_H
