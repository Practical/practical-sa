/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "function_call.h"

#include "ast/expression/identifier.h"

#include <practical-errors.h>

using namespace PracticalSemanticAnalyzer;

namespace AST::ExpressionImpl {

FunctionCall::FunctionCall( const NonTerminals::Expression::FunctionCall &parserFunctionCall ) :
    parserFunctionCall( parserFunctionCall )
{
}

// protected methods
void FunctionCall::buildASTImpl( LookupContext &lookupContext, ExpectedResult expectedResult ) {
    functionId.emplace( *parserFunctionCall.expression );
    functionId->buildAST( lookupContext, ExpectedResult() );

    const Identifier *identifier = functionId->tryGetActualExpression<Identifier>();
    ASSERT(identifier)<<"TODO calling function through generic pointer expression not yet implemented";

    struct Visitor {
        FunctionCall *_this;
        LookupContext &lookupContext;
        ExpectedResult &expectedResult;

        void operator()( const LookupContext::Variable &var ) {
            ABORT()<<"TODO calling function through a variable not yet implemented";
        }

        void operator()( const LookupContext::Function &function ) {
            _this->resolveOverloads( lookupContext, expectedResult, function.overloads );
        }
    };

    std::visit(
            Visitor{ ._this=this, .lookupContext=lookupContext, .expectedResult=expectedResult },
            *identifier->getCtxIdentifier() );
}

ExpressionId FunctionCall::codeGenImpl( PracticalSemanticAnalyzer::FunctionGen *functionGen ) {
    return definition->codeGen( arguments, definition, functionGen );
}

// Private
void FunctionCall::resolveOverloads(
        LookupContext &lookupContext,
        ExpectedResult expectedResult,
        const std::unordered_map<const Tokenizer::Token *, LookupContext::Function::Definition> &overloads
        )
{
    std::vector< const LookupContext::Function::Definition * > relevantOverloads;
    std::vector< const LookupContext::Function::Definition * > preciseResultOverloads;

    size_t numArguments = parserFunctionCall.arguments.arguments.size();
    for( auto &overload : overloads ) {
        auto overloadType = std::get<const StaticType::Function *>(overload.second.type->getType());
        if( overloadType->getNumArguments() == numArguments ) {
            relevantOverloads.emplace_back( &overload.second );

            if( expectedResult && expectedResult.getType() == overloadType->getReturnType() )
                preciseResultOverloads.emplace_back( &overload.second );
        }
    }

    if( relevantOverloads.size()==0 ) {
        throw NoMatchingOverload( parserFunctionCall.op );
    }

    if( relevantOverloads.size()==1 ) {
        // It's the only one that might match. Either it matches or compile error.
        buildActualCall( lookupContext, expectedResult, relevantOverloads[0] );
        return;
    }

    ASSERT( preciseResultOverloads.size()==1 )<<"TODO actual overload _resolution_ is not yet implemented";

    buildActualCall( lookupContext, expectedResult, preciseResultOverloads[0] );
}

void FunctionCall::buildActualCall(
            LookupContext &lookupContext, ExpectedResult expectedResult,
            const LookupContext::Function::Definition *definition )
{
    auto functionType = std::get<const StaticType::Function *>( definition->type->getType() );
    size_t numArguments = functionType->getNumArguments();
    ASSERT( numArguments==parserFunctionCall.arguments.arguments.size() );

    arguments.reserve( numArguments );

   for( unsigned argumentNum=0; argumentNum<numArguments; ++argumentNum ) {
        Expression &argument = arguments.emplace_back( parserFunctionCall.arguments.arguments[argumentNum] );
        argument.buildAST( lookupContext, ExpectedResult( functionType->getArgumentType(argumentNum) ) );
    }

    StaticTypeImpl::CPtr returnType = static_cast<const StaticTypeImpl *>( functionType->getReturnType().get() );
    metadata.valueRange = returnType->defaultRange();
    metadata.type = std::move(returnType);

    this->definition = definition;
}

} // namespace AST::ExpressionImpl
