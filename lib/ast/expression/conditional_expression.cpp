/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/expression/conditional_expression.h"
#include "ast/ast.h"

namespace AST::ExpressionImpl {

ConditionalExpression::ConditionalExpression( const NonTerminals::ConditionalExpression &parserCondition ) :
    condition(parserCondition.condition),
    ifClause(parserCondition.ifClause),
    elseClause(parserCondition.elseClause)
{}

void ConditionalExpression::buildASTImpl(
        LookupContext &lookupContext, ExpectedResult expectedResult, unsigned &weight, unsigned weightLimit )
{
    unsigned conditionWeight = 0;
    auto boolType = AST::getBuiltinCtx().lookupType("Bool");
    condition.buildAST(lookupContext, boolType, conditionWeight, Expression::NoWeightLimit);

    ifClause.buildAST(lookupContext, expectedResult, weight, weightLimit);
    elseClause.buildAST(lookupContext, ifClause.getType(), weight, weightLimit);

    metadata.type = ifClause.getType();
    metadata.valueRange = metadata.type->defaultRange(); // TODO merge the two ranges instead
}

ExpressionId ConditionalExpression::codeGenImpl( PracticalSemanticAnalyzer::FunctionGen *functionGen ) const {
    ExpressionId conditionResult = condition.codeGen(functionGen);
    JumpPointId elsePoint{ jumpPointAllocator.allocate() }, contPoint{ jumpPointAllocator.allocate() };

    ExpressionId resultId = allocateId();
    functionGen->conditionalBranch( resultId, ifClause.getType(), conditionResult, elsePoint, contPoint );

    functionGen->setConditionClauseResult( ifClause.codeGen( functionGen ) );

    functionGen->setJumpPoint( elsePoint );
    functionGen->setConditionClauseResult( elseClause.codeGen( functionGen ) );

    functionGen->setJumpPoint( contPoint );

    return resultId;
}

} // namespace AST::ExpressionImpl
