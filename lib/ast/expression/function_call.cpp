/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "function_call.h"

using namespace PracticalSemanticAnalyzer;

namespace AST::ExpressionImpl {

FunctionCall::FunctionCall( const NonTerminals::Expression::FunctionCall &parserFunctionCall ) :
    parserFunctionCall( parserFunctionCall )
{
}

void FunctionCall::buildAST( LookupContext &lookupContext, ExpectedResult expectedResult ) {
    functionId.emplace( *parserFunctionCall.expression );
    functionId->buildAST( lookupContext, ExpectedResult() );

    StaticType::Types functionIdType = functionId->getType()->getType();
    auto functionType = std::get_if<const StaticType::Function *>( &functionIdType );

    ASSERT( functionType!=nullptr )<<"TODO implement calling non function identifiers";

    functionName = (*functionType)->getFunctionName();
    ASSERT( arguments.size()==0 )<<"buildAST called twice";
    arguments.reserve( (*functionType)->getNumArguments() );

    for( unsigned argumentNum=0; argumentNum<(*functionType)->getNumArguments(); ++argumentNum ) {
        Expression &argument = arguments.emplace_back( parserFunctionCall.arguments.arguments[argumentNum] );
        argument.buildAST( lookupContext, ExpectedResult( (*functionType)->getArgumentType(argumentNum) ) );
    }

    if( !expectedResult || *expectedResult.getType() == *(*functionType)->getReturnType() ) {
        returnType = (*functionType)->getReturnType();
        return;
    }

    ABORT()<<"TODO implement casting result";
}

ExpressionId FunctionCall::codeGen( PracticalSemanticAnalyzer::FunctionGen *functionGen ) {
    ASSERT( functionName )<<"TODO implement overloads and callable expressions";

    ExpressionId resultId = Expression::allocateId();
    ExpressionId argumentExpressionIds[arguments.size()];
    for( unsigned argIdx=0; argIdx<arguments.size(); ++argIdx ) {
        argumentExpressionIds[argIdx] = arguments[argIdx].codeGen( functionGen );
    }

    functionGen->callFunctionDirect(
            resultId, functionName, Slice(argumentExpressionIds, arguments.size()), returnType );

    return resultId;
}

} // namespace AST::ExpressionImpl
