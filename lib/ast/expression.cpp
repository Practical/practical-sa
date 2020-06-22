/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/expression.h"

#include "ast/expression/binary_op.h"
#include "ast/expression/compound_expression.h"
#include "ast/expression/conditional_expression.h"
#include "ast/expression/identifier.h"
#include "ast/expression/function_call.h"
#include "ast/expression/literal.h"

using namespace PracticalSemanticAnalyzer;

namespace AST {

Expression::Expression( const NonTerminals::Expression &parserExpression ) :
    parserExpression( parserExpression )
{
}

void Expression::buildASTImpl(
        LookupContext &lookupContext, ExpectedResult expectedResult, unsigned &weight, unsigned weightLimit )
{
    struct Visitor {
        Expression *_this;
        LookupContext &lookupContext;
        ExpectedResult expectedResult;
        unsigned &weight;
        const unsigned weightLimit;

        void operator()( const std::unique_ptr<NonTerminals::CompoundExpression> &parserExpression ) {
            auto expression = safenew<ExpressionImpl::CompoundExpression>( *parserExpression, lookupContext );

            expression->buildAST( lookupContext, expectedResult, weight, weightLimit );
            _this->actualExpression = std::move(expression);
        }

        void operator()( const NonTerminals::Literal &parserLiteral ) {
            auto literal = safenew<ExpressionImpl::Literal>( parserLiteral );

            literal->buildAST( lookupContext, expectedResult, weight, weightLimit );
            _this->actualExpression = std::move(literal);
        }

        void operator()( const NonTerminals::Identifier &parserIdentifier ) {
            auto identifier = safenew<ExpressionImpl::Identifier>( parserIdentifier );

            identifier->buildAST( lookupContext, expectedResult, weight, weightLimit );
            _this->actualExpression = std::move(identifier);
        }

        void operator()( const NonTerminals::Expression::UnaryOperator &op ) {
            ABORT()<<"TODO implement";
        }

        void operator()( const NonTerminals::Expression::BinaryOperator &op ) {
            auto binaryOp = safenew<ExpressionImpl::BinaryOp>(op);

            binaryOp->buildAST( lookupContext, expectedResult, weight, weightLimit );
            _this->actualExpression = std::move(binaryOp);
        }

        void operator()( const NonTerminals::Expression::FunctionCall &parserFuncCall ) {
            auto functionCall = safenew<ExpressionImpl::FunctionCall>( parserFuncCall );

            functionCall->buildAST( lookupContext, expectedResult, weight, weightLimit );
            _this->actualExpression = std::move(functionCall);
        }

        void operator()( const std::unique_ptr<NonTerminals::ConditionalExpression> &parserCondition ) {
            auto condition = safenew<ExpressionImpl::ConditionalExpression>( *parserCondition );

            condition->buildAST( lookupContext, expectedResult, weight, weightLimit );
            _this->actualExpression = std::move(condition);
        }

        void operator()( const NonTerminals::Type &type ) {
            ABORT()<<"TODO implement";
        }
    };

    std::visit(
            Visitor{
                ._this = this, .lookupContext = lookupContext, .expectedResult = expectedResult,
                .weight = weight, .weightLimit = weightLimit
            },
            parserExpression.value );

    metadata.type = actualExpression->getType();
    metadata.valueRange = actualExpression->getValueRange();
}

ExpressionId Expression::codeGenImpl( PracticalSemanticAnalyzer::FunctionGen *functionGen ) const {
    return actualExpression->codeGen( functionGen );
}

} // namespace AST
