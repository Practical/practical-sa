/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/ast.h"
#include "ast/conditional_statement.h"
#include "ast/statement.h"

namespace AST {

ConditionalStatement::ConditionalStatement( const NonTerminals::Statement::ConditionalStatement &parserCondition ) :
    condition( parserCondition.condition )
{
    ASSERT( parserCondition.ifClause );
    ifClause = safenew<Statement>( *parserCondition.ifClause );

    if( parserCondition.elseClause ) {
        elseClause = safenew<Statement>( *parserCondition.elseClause );
    }
}

void ConditionalStatement::buildAST( LookupContext &lookupCtx ) {
    Weight weight;
    auto boolType = AST::getBuiltinCtx().lookupType("Bool");
    condition.buildAST(lookupCtx, boolType, weight, Expression::NoWeightLimit);
    ifClause->buildAST(lookupCtx);
    if( elseClause )
        elseClause->buildAST(lookupCtx);
}

void ConditionalStatement::codeGen(
        const LookupContext &lookupCtx, PracticalSemanticAnalyzer::FunctionGen *functionGen ) const
{
    ExpressionId conditionResult = condition.codeGen(functionGen);
    JumpPointId elsePoint, contPoint;
    if( elseClause ) {
        elsePoint = jumpPointAllocator.allocate();
    }
    // It's a silly thing to do, but make sure that the continuation jump point it higher than the else jump point
    contPoint = jumpPointAllocator.allocate();

    functionGen->conditionalBranch( ExpressionId(), StaticType::CPtr(), conditionResult, elsePoint, contPoint );

    ifClause->codeGen( lookupCtx, functionGen );

    if( elseClause ) {
        functionGen->setJumpPoint( elsePoint );
        elseClause->codeGen( lookupCtx, functionGen );
    }

    functionGen->setJumpPoint( contPoint );
}

} // namespace AST
