/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/casts.h"

#include "ast/expression.h"
#include "ast/signed_int_value_range.h"
#include "ast/unsigned_int_value_range.h"

namespace AST {

ExpressionId signedExpansionCast( 
            PracticalSemanticAnalyzer::StaticType::CPtr sourceType, ExpressionId sourceExpression,
            PracticalSemanticAnalyzer::StaticType::CPtr destType,
            PracticalSemanticAnalyzer::FunctionGen *functionGen)
{
    ExpressionId id = Expression::allocateId();
    functionGen->expandIntegerSigned( id, sourceExpression, sourceType, destType );

    return id;
}

ExpressionId unsignedExpansionCast( 
            PracticalSemanticAnalyzer::StaticType::CPtr sourceType, ExpressionId sourceExpression,
            PracticalSemanticAnalyzer::StaticType::CPtr destType,
            PracticalSemanticAnalyzer::FunctionGen *functionGen)
{
    ExpressionId id = Expression::allocateId();
    functionGen->expandIntegerUnsigned( id, sourceExpression, sourceType, destType );

    return id;
}

ValueRangeBase::CPtr identityVrp(
            const StaticTypeImpl *sourceType,
            const StaticTypeImpl *destType,
            ValueRangeBase::CPtr inputRange )
{
    return inputRange;
}

ValueRangeBase::CPtr unsignedToSignedIdentityVrp(
            const StaticTypeImpl *sourceType,
            const StaticTypeImpl *destType,
            ValueRangeBase::CPtr inputRangeBase )
{
    ASSERT(
            std::get<const StaticType::Scalar *>(sourceType->getType())->getType() ==
            StaticType::Scalar::Type::UnsignedInt
          ) <<
            "VRP for unsigned->signed called on input of type "<<
            std::get<const StaticType::Scalar *>(sourceType->getType())->getType();

    ASSERT( dynamic_cast<const UnsignedIntValueRange *>(inputRangeBase.get())!=nullptr );

    auto inputRange = static_cast<const UnsignedIntValueRange *>(inputRangeBase.get());

    auto maximalRange = destType->defaultRange();
    ASSERT( dynamic_cast<const SignedIntValueRange *>( maximalRange.get() )!=nullptr );

    ASSERT(
            inputRange->maximum <=
            static_cast<LongEnoughInt>( static_cast<const SignedIntValueRange *>(maximalRange.get())->maximum ) );

    return SignedIntValueRange::allocate( inputRange->minimum, inputRange->maximum );
}

} // namespace AST
