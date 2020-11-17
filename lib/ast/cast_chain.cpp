/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/cast_chain.h"

#include "ast/expression/base.h"
#include "ast/decay.h"

#include <practical/errors.h>

using namespace PracticalSemanticAnalyzer;

namespace AST {

static std::vector< StaticTypeImpl::CPtr > castCandidates(
        const LookupContext &lookupContext,
        std::unordered_map< StaticTypeImpl::CPtr, CastChain::Junction > &paths,
        StaticTypeImpl::CPtr type );

CastChain::CastChain(
        std::unique_ptr<CastChain> &&previousCast,
        const LookupContext::CastDescriptor &cast,
        const ExpressionImpl::ExpressionMetadata &metadata
    ) :
        previousCast( std::move(previousCast) ),
        cast(cast),
        metadata(metadata)
{}

CastChain::CastChain(
        std::unique_ptr<CastChain> &&previousCast,
        const LookupContext::CastDescriptor &cast
    ) :
        CastChain( std::move(previousCast), cast, { .type = cast.destType } )
{}

std::unique_ptr<CastChain> CastChain::allocate(
        const LookupContext &lookupContext,
        StaticTypeImpl::CPtr destinationType,
        const ExpressionImpl::ExpressionMetadata &srcMetadata,
        Weight &weight, Weight weightLimit,
        bool implicit, size_t line, size_t col )
{
    ASSERT( implicit )<<"TODO explicit cast is untested";
    ASSERT( destinationType != srcMetadata.type )<<
            "No point in seeking path from "<<srcMetadata.type<<" to "<<destinationType;

    {
        // Fastpath: direct cast from source to destination
        auto castDescriptor = lookupContext.lookupCast( srcMetadata.type, destinationType );

        if(
                castDescriptor!=nullptr &&
                castDescriptor->whenPossible!=LookupContext::CastDescriptor::ImplicitCastAllowed::Never )
        {
            if( weight+Weight(castDescriptor->weight) > weightLimit ) {
                throw ExpressionImpl::Base::ExpressionTooExpensive();
            }
            weight += Weight(castDescriptor->weight);
            return fastPathAllocate( castDescriptor, srcMetadata );
        }
    }

    std::unordered_map< StaticTypeImpl::CPtr, Junction > paths;

    std::vector< StaticTypeImpl::CPtr > pendingCandidates, candidates;

    paths.emplace( srcMetadata.type, Junction{ .pathWeight = weight } );
    pendingCandidates.push_back( srcMetadata.type );

    std::vector< const Junction * > validPaths;

    bool possiblyTooExpensive = false;

    do {
        candidates = std::move( pendingCandidates );
        ASSERT( pendingCandidates.empty() );

        for( auto candidate : candidates ) {
            const Junction &path = paths.at( candidate );

            if( path.pathWeight>weightLimit ) {
                possiblyTooExpensive = true;
                continue;
            }

            if( *candidate == *destinationType ) {
                if( path.pathWeight < weightLimit ) {
                    weightLimit = path.pathWeight;
                    validPaths.clear();
                }

                validPaths.emplace_back( &path );
                continue;
            }

            auto temporaryCandidates = decay( paths, candidate );
            pendingCandidates.insert(
                    pendingCandidates.end(), temporaryCandidates.begin(), temporaryCandidates.end() );
            temporaryCandidates = castCandidates( lookupContext, paths, candidate );
            pendingCandidates.insert(
                    pendingCandidates.end(), temporaryCandidates.begin(), temporaryCandidates.end() );
        }
    } while(! pendingCandidates.empty());

    if( validPaths.empty() ) {
        if( possiblyTooExpensive )
            throw ExpressionImpl::Base::ExpressionTooExpensive();

        return nullptr;
    }

    if( validPaths.size()>1 )
        throw AmbiguousCast(srcMetadata.type, destinationType, implicit, line, col);

    const Junction *currentJunction = validPaths[0];
    std::unique_ptr<CastChain> ret( new CastChain( nullptr, *currentJunction->descriptor ) );
    CastChain *currentChain = ret.get();
    StaticTypeImpl::CPtr currentType = destinationType;

    goto skipAllocation;

    while( currentJunction->predecessor ) {
        currentChain->previousCast = std::unique_ptr<CastChain>(
                new CastChain(nullptr, *currentJunction->descriptor) );
        currentChain = currentChain->previousCast.get();

skipAllocation:
        currentChain->metadata.type = currentType;

        currentType = currentJunction->predecessor;
        currentJunction = &paths.at(currentType);
    }

    try {
        ret->calcVrp( srcMetadata, implicit, line, col );
    } catch( CastNotAllowed &ex ) {
        return nullptr;
    }

    ASSERT( ret );

    weight = weightLimit;
    return ret;
}

ExpressionId CastChain::codeGen(
            PracticalSemanticAnalyzer::StaticType::CPtr sourceType, ExpressionId sourceExpression,
            PracticalSemanticAnalyzer::FunctionGen *functionGen
        ) const
{
    ExpressionId previousResult = sourceExpression;
    if( previousCast ) {
        previousResult = previousCast->codeGen( sourceType, sourceExpression, functionGen );
        sourceType = previousCast->getMetadata().type;
    }

    return cast.codeGen( sourceType, previousResult, metadata.type, functionGen );
}

std::unique_ptr<CastChain> CastChain::fastPathAllocate(
            const LookupContext::CastDescriptor *castDescriptor,
            const ExpressionImpl::ExpressionMetadata &srcMetadata )
{
    ExpressionImpl::ExpressionMetadata metadata;
    metadata.type = castDescriptor->destType;

    if( castDescriptor->calcVrp ) {
        metadata.valueRange = castDescriptor->calcVrp(
                srcMetadata.type.get(),
                castDescriptor->destType.get(),
                srcMetadata.valueRange,
                true );

        if( !metadata.valueRange )
            return nullptr;

    } else {
        ASSERT( castDescriptor->whenPossible == LookupContext::CastDescriptor::ImplicitCastAllowed::Always );

        metadata.valueRange = castDescriptor->destType->defaultRange();
    }

    return std::unique_ptr<CastChain>( new CastChain(
                nullptr,
                *castDescriptor,
                std::move( metadata ) )
            );
}

static std::vector< StaticTypeImpl::CPtr > castCandidates(
        const LookupContext &lookupContext,
        std::unordered_map< StaticTypeImpl::CPtr, CastChain::Junction > &paths,
        StaticTypeImpl::CPtr type )
{
    std::vector< StaticTypeImpl::CPtr > ret;

    auto weight = paths.at(type).pathWeight;
    for( auto candidates = lookupContext.allCastsFrom(type); candidates; ++candidates ) {
        ASSERT( candidates->destType );
        auto pathWeight = weight + Weight( candidates->weight );
        auto previousIter = paths.find( candidates->destType );
        if( previousIter!=paths.end() ) {
            if( pathWeight > previousIter->second.pathWeight )
                continue;
            if( pathWeight == previousIter->second.pathWeight ) {
                previousIter->second.multiplePaths = true;
                continue;
            }

            paths.erase( previousIter );
        } else {
            ret.push_back( candidates->destType );
        }

        paths.emplace( candidates->destType, CastChain::Junction{
                .descriptor = &*candidates,
                .predecessor = candidates->sourceType,
                .pathWeight = pathWeight,
            } );
    }

    return ret;
}

void CastChain::calcVrp(
        const ExpressionImpl::ExpressionMetadata &srcMetadata, bool isImplicit, size_t line, size_t col )
{
    ASSERT( ! metadata.valueRange );

    const ExpressionImpl::ExpressionMetadata *prevMetadata = nullptr;
    if( previousCast ) {
        previousCast->calcVrp( srcMetadata, isImplicit, line, col );

        prevMetadata = &previousCast->metadata;
    } else {
        prevMetadata = &srcMetadata;
    }

    metadata.valueRange = cast.calcVrp(
            prevMetadata->type.get(),
            metadata.type.get(),
            prevMetadata->valueRange,
            isImplicit );

    if( metadata.valueRange == nullptr )
        throw CastNotAllowed( prevMetadata->type, metadata.type, isImplicit, line, col );
}

} // namespace AST
