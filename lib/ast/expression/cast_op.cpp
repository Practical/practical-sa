/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/expression/cast_op.h"

namespace AST::ExpressionImpl {

CastOp::CastOp( const NonTerminals::Expression::CastOperator &parserCast ) :
    parserCast(parserCast),
    expression(*parserCast.expression)
{
}

SourceLocation CastOp::getLocation() const {
    return parserCast.op->location;
}

void CastOp::buildASTImpl(
        LookupContext &lookupContext, ExpectedResult expectedResult, Weight &weight, Weight weightLimit
    )
{
    metadata.type = lookupContext.lookupType( parserCast.destType );

    switch( parserCast.op->token ) {
    case Tokenizer::Tokens::RESERVED_EXPECT:
        {
            // Just give the expression a mandatory expected type
            expression.buildAST( lookupContext, metadata.type, weight, weightLimit );
            metadata.valueRange = expression.getValueRange();
        }
        break;
    default:
        ABORT()<<"Unidentified token "<<parserCast.op->token<<" passed as cast";
    }
}

ExpressionId CastOp::codeGenImpl( PracticalSemanticAnalyzer::FunctionGen *functionGen ) const {
    return expression.codeGen( functionGen );
}

} // namespace AST::ExpressionImpl
