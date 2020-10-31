/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/decay.h"

#include "ast/expression/base.h"

namespace AST {

static ExpressionId referenceToBuiltinValueCodegen(
        PracticalSemanticAnalyzer::StaticType::CPtr sourceType, ExpressionId sourceExpression,
        PracticalSemanticAnalyzer::StaticType::CPtr destType,
        PracticalSemanticAnalyzer::FunctionGen *functionGen)
{
    ExpressionId resultId = ExpressionImpl::Base::allocateId();

    functionGen->dereferencePointer( resultId, destType, sourceExpression );

    return resultId;
}

ValueRangeBase::CPtr referenceToBuiltinValueVrp(
            const StaticTypeImpl *sourceType,
            const StaticTypeImpl *destType,
            ValueRangeBase::CPtr inputRange,
            bool isImplicit
        )
{
    ASSERT( *sourceType->removeFlags( StaticType::Flags::Reference ) == *destType ) <<
            "Cannot decay from "<<*sourceType<<" to "<<*destType;

    return inputRange;
}

enum class DecayCasts {
    ReferenceToBuiltinValue
};

static const std::unordered_map< DecayCasts, LookupContext::CastDescriptor > decayCasts{
    {
        DecayCasts::ReferenceToBuiltinValue,
        LookupContext::CastDescriptor(
                nullptr, nullptr, referenceToBuiltinValueCodegen, referenceToBuiltinValueVrp, 0,
                LookupContext::CastDescriptor::ImplicitCastAllowed::Always )
    }
};

void decayInit() {
}

std::vector< StaticTypeImpl::CPtr > decay(
            std::unordered_map< StaticTypeImpl::CPtr, CastChain::Junction > &paths,
            StaticTypeImpl::CPtr type, unsigned length, unsigned weight )
{
    using RetType = std::vector< StaticTypeImpl::CPtr >;
    if( (type->getFlags() & StaticTypeImpl::Flags::Reference) != 0 ) {
        auto decayedType = downCast(type->removeFlags( StaticTypeImpl::Flags::Reference ));
        paths.emplace(
                decayedType,
                CastChain::Junction{
                    .descriptor = &decayCasts.at(DecayCasts::ReferenceToBuiltinValue),
                    .pathWeight=weight+1, .length=length }
            );
        return RetType{ decayedType };
    }

    return RetType();
}

} // namespace AST
