/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/expression/compound_expression.h"

namespace AST::ExpressionImpl {

CompoundExpression::CompoundExpression(
        const NonTerminals::CompoundExpression &parserExpression, const LookupContext &parentCtx ) :
    lookupContext( &parentCtx ),
    statements(parserExpression.statementList),
    expression(parserExpression.expression)
{
}

size_t CompoundExpression::getLine() const {
    return expression.getLine();
}

size_t CompoundExpression::getCol() const {
    return expression.getCol();
}

void CompoundExpression::buildASTImpl(
        LookupContext &lookupContext, ExpectedResult expectedResult, unsigned &weight, unsigned weightLimit)
{
    ASSERT( &lookupContext == this->lookupContext.getParent() );

    statements.buildAST( this->lookupContext );
    expression.buildAST( this->lookupContext, expectedResult, weight, weightLimit );

    metadata.type = expression.getType();
    metadata.valueRange = expression.getValueRange();
}

ExpressionId CompoundExpression::codeGenImpl( PracticalSemanticAnalyzer::FunctionGen *functionGen ) const {
    statements.codeGen( this->lookupContext, functionGen );
    return expression.codeGen( functionGen );
}

} // namespace AST::ExpressionImpl
