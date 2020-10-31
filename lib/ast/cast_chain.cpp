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

#include <practical-errors.h>

using namespace PracticalSemanticAnalyzer;

namespace AST {

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
        unsigned &weight, unsigned weightLimit,
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
            if( weight+castDescriptor->weight > weightLimit ) {
                throw ExpressionImpl::Base::ExpressionTooExpensive();
            }
            weight += castDescriptor->weight;
            return fastPathAllocate( castDescriptor, srcMetadata );
        }
    }

    std::unordered_map< StaticTypeImpl::CPtr, Junction > paths;

    std::vector< StaticTypeImpl::CPtr > pendingCandidates, candidates;

    // Check decay options
    pendingCandidates = decay( paths, srcMetadata.type, 0, 0 );
    for( auto &pathNode : paths ) {
        if( pathNode.first != destinationType )
            continue;

        // Our type directly decays into the destination type
        weight += pathNode.second.pathWeight;
        if( weight>weightLimit )
            throw ExpressionImpl::Base::ExpressionTooExpensive();

        ExpressionImpl::ExpressionMetadata metadata;
        metadata.type = destinationType;
        metadata.valueRange = pathNode.second.descriptor->calcVrp(
                srcMetadata.type.get(), destinationType.get(), srcMetadata.valueRange, true );

        return std::unique_ptr<CastChain>( new CastChain( nullptr, *pathNode.second.descriptor, metadata ) );
    }

    LookupContext::CastsList upperHook = lookupContext.allCastsTo( destinationType );
    if( !upperHook && pendingCandidates.empty() )
        return nullptr;

    /* We are looking for the shortest (least number of casts) path between sourceType and destinationType,
     * that *also* has the least weight. To that end, we are doing a breadth first search (BFS) of the graph
     * of all possible casts.
     *
     * Future enhancement:
     * Since BFS trees tend to be rather wide, we minimize the search by doing two searches, one starting with
     * sourceType, and another ending at destinationType. Any point the two trees meet is a valid sequence of
     * casts.
     *
     * We start with all casts leading to destinationType (upper tree). If not found, we search the lower tree.
     * The casts in the lower tree must be distinct, or the path would have been closed by the upper one.
     * We then continue populating the upper tree in the casts leading to the types covered by the previous step
     * and so on until we run out of new types to reach or we find a path.
     */

    paths.emplace( destinationType, Junction{ .pathWeight=weight, .length=0 } );

    unsigned effectiveLimit = ExpressionImpl::Base::NoWeightLimit;
    for( ; upperHook ; ++upperHook ) {
        ASSERT( upperHook->sourceType != srcMetadata.type )<<
                "Single step cast should have been handled by fastpath "<<srcMetadata.type<<" to "<<destinationType;

        if( implicit && upperHook->whenPossible==LookupContext::CastDescriptor::ImplicitCastAllowed::Never )
            continue;

        unsigned interimWeight = upperHook->weight;
        if( interimWeight<=weightLimit ) {
            paths.emplace( upperHook->sourceType, Junction{ .descriptor=upperHook.get(), .pathWeight=interimWeight, .length=1 } );
            pendingCandidates.emplace_back( upperHook->sourceType );
        } else {
            // Since the list is sorted, there will be no valid candidates past this one
            break;
        }
    }

    std::vector< Junction > validPaths;
    unsigned chainLength = 1;
    while( !pendingCandidates.empty() && validPaths.empty() ) {
        candidates = std::move(pendingCandidates);
        ASSERT( pendingCandidates.size()==0 );
        chainLength++;

        for( const auto &candidate : candidates ) {
            upperHook = lookupContext.allCastsTo( candidate );

            for( ; upperHook ; ++upperHook ) {
                if(
                        implicit &&
                        upperHook->whenPossible==LookupContext::CastDescriptor::ImplicitCastAllowed::Never )
                {
                    continue;
                }

                unsigned interimWeight = upperHook->weight + paths.at( upperHook->destType ).pathWeight;

                if( interimWeight>effectiveLimit ) {
                    // Since the list is sorted, there will be no valid candidates past this one
                    break;
                }

                if( upperHook->sourceType == srcMetadata.type ) {
                    if( interimWeight<effectiveLimit ) {
                        // validPaths has paths that are too expensive
                        validPaths.clear();
                    }

                    effectiveLimit = interimWeight;
                    validPaths.emplace_back(
                            Junction{ .descriptor = upperHook.get(), .pathWeight = interimWeight, .length = chainLength } );

                    break;
                }

                auto sourceIter = paths.find( upperHook->sourceType );
                if( sourceIter!=paths.end() ) {
                    // We've already reached this type. See if this time it's a better path
                    if( sourceIter->second.length==chainLength && sourceIter->second.pathWeight>interimWeight ) {
                        sourceIter->second.pathWeight = interimWeight;
                        sourceIter->second.descriptor = upperHook.get();
                    }

                    continue;
                }

                paths.emplace(
                        upperHook->sourceType,
                        Junction{ .descriptor = upperHook.get(), .pathWeight=interimWeight, .length=chainLength } );
                pendingCandidates.emplace_back( upperHook->sourceType );
            }
        }
    }

    if( validPaths.size()>1 )
        throw AmbiguousCast(srcMetadata.type, destinationType, implicit, line, col);

    if( validPaths.empty() )
        return nullptr;

    weight += validPaths[0].pathWeight;
    if( weight>weightLimit )
        throw ExpressionImpl::Base::ExpressionTooExpensive();

    const LookupContext::CastDescriptor *currentCast = validPaths[0].descriptor;
    ASSERT( currentCast!=nullptr );
    ASSERT( currentCast->sourceType == srcMetadata.type );
    std::unique_ptr<CastChain> ret( new CastChain( nullptr, *currentCast ) );

    const ExpressionImpl::ExpressionMetadata *lastMetadata = &srcMetadata;
    goto skipSettingRet;
    do {
        ret = std::unique_ptr<CastChain>( new CastChain( std::move(ret), *currentCast ) );

skipSettingRet:
        ASSERT( currentCast->calcVrp != nullptr );
        ret->metadata.valueRange = currentCast->calcVrp(
                currentCast->sourceType.get(), currentCast->destType.get(),
                lastMetadata->valueRange, implicit );
        if( ! ret->metadata.valueRange )
            return nullptr;

        lastMetadata = &ret->metadata;
        currentCast = paths.at( currentCast->destType ).descriptor;
    } while( currentCast!=nullptr );

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

} // namespace AST
