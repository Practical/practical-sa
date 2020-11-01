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
        Weight &weight,
        Weight weightLimit,
        ExpressionMetadata &metadata,
        Slice<const NonTerminals::Expression *const> parserArguments,
        const Tokenizer::Token *sourceLocation
    )
{
    if( expectedResult ) {
        resolveOverloadsByReturn(
                lookupContext, expectedResult, overloads, weight, weightLimit, metadata, parserArguments,
                sourceLocation );
    } else {
        resolveOverloadsByArguments(
                lookupContext, overloads, weight, weightLimit, metadata, parserArguments, sourceLocation );
    }
}

ExpressionId OverloadResolver::codeGen( PracticalSemanticAnalyzer::FunctionGen *functionGen ) const {
    return definition->codeGen( arguments, definition, functionGen );
}

// Private
void OverloadResolver::buildActualCall(
            LookupContext &lookupContext, Weight &weight, Weight weightLimit,
            const LookupContext::Function::Definition *definition,
            ExpressionMetadata &metadata,
            Slice<const NonTerminals::Expression *const> parserArguments )
{
    auto functionType = std::get<const StaticType::Function *>( definition->type->getType() );
    size_t numArguments = functionType->getNumArguments();
    ASSERT( numArguments==parserArguments.size() );

    arguments.reserve( numArguments );

    for( unsigned argumentNum=0; argumentNum<numArguments; ++argumentNum ) {
        Expression &argument = arguments.emplace_back( *parserArguments[argumentNum] );
        Weight additionalWeight;
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

void OverloadResolver::resolveOverloadsByReturn(
        LookupContext &lookupContext,
        ExpectedResult expectedResult,
        const std::vector<LookupContext::Function::Definition> &overloads,
        Weight &weight,
        Weight weightLimit,
        ExpressionMetadata &metadata,
        Slice<const NonTerminals::Expression *const> parserArguments,
        const Tokenizer::Token *sourceLocation
    )
{
    std::unordered_map<
            StaticType::CPtr,
            std::vector< const LookupContext::Function::Definition * >
        > sortedOverloads;
    size_t numArguments = parserArguments.size();

    for( auto &overload : overloads ) {
        auto overloadType = std::get<const StaticType::Function *>(overload.type->getType());
        if( overloadType->getNumArguments() == numArguments ) {
            sortedOverloads[ overloadType->getReturnType() ].emplace_back( &overload );
        }
    }

    if( sortedOverloads.size()==0 ) {
        throw NoMatchingOverload( sourceLocation );
    }

    if( sortedOverloads.size()==1 && sortedOverloads.begin()->second.size()==1 ) {
        // It's the only one that might match. Either it matches or compile error.
        buildActualCall(
                lookupContext, weight, weightLimit, *sortedOverloads.begin()->second.begin(), metadata, parserArguments );
        return;
    }

    auto currentReturnCandidate = sortedOverloads.find( expectedResult.getType() );
    if( currentReturnCandidate!=sortedOverloads.end() ) {
        ASSERT( ! currentReturnCandidate->second.empty() );

        if( currentReturnCandidate->second.size()==1 ) {
            // Only one overload matches the return type exactly
            buildActualCall(
                    lookupContext, weight, weightLimit, currentReturnCandidate->second[0], metadata, parserArguments );
            return;
        }

        try {
            findBestOverloadByArgument(
                    lookupContext, currentReturnCandidate->second, weight, weightLimit, metadata,
                    parserArguments, sourceLocation );

            return;
        } catch( NoMatchingOverload &ex ) {
        }
    }

    ABORT()<<"TODO implement";

    /*
    buildActualCall(
            lookupContext, expectedResult, weight, weightLimit, preciseResultOverloads[0], metadata, parserArguments );
    */
}

void OverloadResolver::resolveOverloadsByArguments(
        LookupContext &lookupContext,
        const std::vector<LookupContext::Function::Definition> &overloads,
        Weight &weight,
        Weight weightLimit,
        ExpressionMetadata &metadata,
        Slice<const NonTerminals::Expression *const> parserArguments,
        const Tokenizer::Token *sourceLocation
    )
{
    std::vector<const LookupContext::Function::Definition *> relevantOverloads;
    size_t numArguments = parserArguments.size();

    for( auto &overload : overloads ) {
        auto overloadType = std::get<const StaticType::Function *>(overload.type->getType());
        if( overloadType->getNumArguments() == numArguments ) {
            relevantOverloads.emplace_back( &overload );
        }
    }

    if( relevantOverloads.size()==0 ) {
        throw NoMatchingOverload( sourceLocation );
    }

    if( relevantOverloads.size()==1 ) {
        // It's the only one that might match. Either it matches or compile error.
        buildActualCall(
                lookupContext, weight, weightLimit, relevantOverloads[0], metadata, parserArguments );
        return;
    }

    findBestOverloadByArgument(
            lookupContext, relevantOverloads, weight, weightLimit, metadata, parserArguments, sourceLocation );
}

void OverloadResolver::findBestOverloadByArgument(
        LookupContext &lookupContext,
        Slice< const LookupContext::Function::Definition * > overloads,
        Weight &weight,
        Weight weightLimit,
        ExpressionMetadata &metadata,
        Slice<const NonTerminals::Expression *const> parserArguments,
        const Tokenizer::Token *sourceLocation
    )
{
    Weight callWeightLimit = weightLimit - weight;
    Weight bestWeight = Base::NoWeightLimit;
    OverloadResolver bestOverloader;
    ExpressionMetadata bestMetadata;
    std::vector< const LookupContext::Function::Definition * > viableOverloads;

    for( auto overload : overloads ) {
        try {
            OverloadResolver provisoryResolver;
            Weight callWeight;
            ExpressionMetadata callMetadata;

            provisoryResolver.buildActualCall(
                    lookupContext, callWeight, callWeightLimit, overload, callMetadata, parserArguments );

            if( callWeight<bestWeight ) {
                bestWeight = callWeight;
                callWeightLimit = callWeight;
                viableOverloads.clear();
                bestOverloader = std::move( provisoryResolver );
                bestMetadata = std::move( callMetadata );
            }

            viableOverloads.emplace_back( overload );
        } catch( NoMatchingOverload &ex ) {
        } catch( AmbiguousOverloads &ex ) {
        } catch( CastError &ex ) {
        } catch( Base::ExpressionTooExpensive &ex ) {
        }
    }

    if( viableOverloads.empty() )
        throw NoMatchingOverload( sourceLocation );

    if( viableOverloads.size()>1 )
        throw AmbiguousOverloads( sourceLocation );

    weight += bestWeight;
    ASSERT( weight<=weightLimit );

    (*this) = std::move( bestOverloader );
    metadata = std::move( bestMetadata );
}

} // namespace AST::ExpressionImpl
