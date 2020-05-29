/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_EXPRESSION_BINARY_OP_H
#define AST_EXPRESSION_BINARY_OP_H

#include "ast/expression.h"
#include "parser.h"

namespace AST::ExpressionImpl {

class BinaryOp final : public Base {
    const NonTerminals::Expression::BinaryOperator &parserOp;
    std::optional<Expression> operands[2];

public:
    static void init(LookupContext &builtinCtx);

    explicit BinaryOp( const NonTerminals::Expression::BinaryOperator &parserOp );

protected:
    void buildASTImpl(
            LookupContext &lookupContext, ExpectedResult expectedResult, unsigned &weight, unsigned weightLimit
        ) override;
    ExpressionId codeGenImpl( PracticalSemanticAnalyzer::FunctionGen *functionGen ) override;
};

} // namespace AST::ExpressionImpl

#endif // AST_EXPRESSION_BINARY_OP_H
