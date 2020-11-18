/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_EXPRESSION_COMPOUND_EXPRESSION_H
#define AST_EXPRESSION_COMPOUND_EXPRESSION_H

#include "ast/expression/base.h"
#include "ast/expression.h"
#include "ast/statement.h"
#include "ast/statement_list.h"
#include "parser.h"

namespace AST::ExpressionImpl {

class CompoundExpression final : public Base {
    LookupContext lookupContext;
    StatementList statements;
    Expression expression;

public:
    CompoundExpression( const NonTerminals::CompoundExpression &parserExpression, const LookupContext &parentCtx );

    SourceLocation getLocation() const override;

protected:
    void buildASTImpl(
            LookupContext &lookupContext, ExpectedResult expectedResult, Weight &weight, Weight weightLimit
        ) override;
    ExpressionId codeGenImpl( PracticalSemanticAnalyzer::FunctionGen *functionGen ) const override;
};

} // namespace AST::ExpressionImpl

#endif // AST_EXPRESSION_COMPOUND_EXPRESSION_H
