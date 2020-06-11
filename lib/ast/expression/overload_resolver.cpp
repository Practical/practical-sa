/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/expression/overload_resolver.h"

#include "ast/expression.h"

#include <practical-errors.h>

namespace AST::ExpressionImpl {

void OverloadResolver::resolveOverloads(
        LookupContext &lookupContext,
        ExpectedResult expectedResult,
        const std::vector<LookupContext::Function::Definition> &overloads,
        unsigned &weight,
        unsigned weightLimit,
        Base::ExpressionMetadata &metadata,
        Slice<const NonTerminals::Expression *const> parserArguments,
        const Tokenizer::Token *sourceLocation
    )
{
    std::vector< const LookupContext::Function::Definition * > relevantOverloads;
    std::vector< const LookupContext::Function::Definition * > preciseResultOverloads;

    size_t numArguments = parserArguments.size();
    for( auto &overload : overloads ) {
        auto overloadType = std::get<const StaticType::Function *>(overload.type->getType());
        if( overloadType->getNumArguments() == numArguments ) {
            relevantOverloads.emplace_back( &overload );

            if( expectedResult && expectedResult.getType() == overloadType->getReturnType() )
                preciseResultOverloads.emplace_back( &overload );
        }
    }

    if( relevantOverloads.size()==0 ) {
        throw NoMatchingOverload( sourceLocation );
    }

    if( relevantOverloads.size()==1 ) {
        // It's the only one that might match. Either it matches or compile error.
        buildActualCall(
                lookupContext, expectedResult, weight, weightLimit, relevantOverloads[0], metadata, parserArguments );
        return;
    }

    ASSERT( preciseResultOverloads.size()==1 )<<"TODO actual overload _resolution_ is not yet implemented";

    buildActualCall(
            lookupContext, expectedResult, weight, weightLimit, preciseResultOverloads[0], metadata, parserArguments );
}

ExpressionId OverloadResolver::codeGen( PracticalSemanticAnalyzer::FunctionGen *functionGen ) {
    return definition->codeGen( arguments, definition, functionGen );
}

// Private
void OverloadResolver::buildActualCall(
            LookupContext &lookupContext, ExpectedResult expectedResult, unsigned &weight, unsigned weightLimit,
            const LookupContext::Function::Definition *definition,
            Base::ExpressionMetadata &metadata,
            Slice<const NonTerminals::Expression *const> parserArguments )
{
    auto functionType = std::get<const StaticType::Function *>( definition->type->getType() );
    size_t numArguments = functionType->getNumArguments();
    ASSERT( numArguments==parserArguments.size() );

    arguments.reserve( numArguments );

    for( unsigned argumentNum=0; argumentNum<numArguments; ++argumentNum ) {
        Expression &argument = arguments.emplace_back( *parserArguments[argumentNum] );
        unsigned additionalWeight = 0;
        argument.buildAST(
                lookupContext, ExpectedResult( functionType->getArgumentType(argumentNum) ),
                additionalWeight, weightLimit );
        ASSERT( additionalWeight<=weightLimit );
        weight+=additionalWeight;
        weightLimit-=additionalWeight;
    }

    StaticTypeImpl::CPtr returnType = static_cast<const StaticTypeImpl *>( functionType->getReturnType().get() );
    if( definition->calcVrp ) {
        ValueRangeBase::CPtr inputRanges[ numArguments ];
        for( unsigned argumentNum=0; argumentNum<numArguments; ++argumentNum ) {
            inputRanges[argumentNum] = arguments[argumentNum].getValueRange();
        }

        metadata.valueRange = definition->calcVrp( definition->type, Slice( inputRanges, numArguments ) );
    } else {
        metadata.valueRange = returnType->defaultRange();
    }
    metadata.type = std::move(returnType);

    this->definition = definition;
}

} // namespace AST::ExpressionImpl
