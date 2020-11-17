/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_CASTS_H
#define AST_CASTS_H

#include "ast/static_type.h"
#include "ast/value_range_base.h"

#include <practical/practical.h>

namespace AST {

using PracticalSemanticAnalyzer::ExpressionId;

ExpressionId signedExpansionCast( 
            PracticalSemanticAnalyzer::StaticType::CPtr sourceType, ExpressionId sourceExpression,
            PracticalSemanticAnalyzer::StaticType::CPtr destType,
            PracticalSemanticAnalyzer::FunctionGen *functionGen);

ExpressionId unsignedExpansionCast( 
            PracticalSemanticAnalyzer::StaticType::CPtr sourceType, ExpressionId sourceExpression,
            PracticalSemanticAnalyzer::StaticType::CPtr destType,
            PracticalSemanticAnalyzer::FunctionGen *functionGen);

ValueRangeBase::CPtr identityVrp(
            const StaticTypeImpl *sourceType,
            const StaticTypeImpl *destType,
            ValueRangeBase::CPtr inputRange,
            bool isImplicit
        );

ValueRangeBase::CPtr unsignedToSignedIdentityVrp(
            const StaticTypeImpl *sourceType,
            const StaticTypeImpl *destType,
            ValueRangeBase::CPtr inputRange,
            bool isImplicit
        );

ExpressionId integerReductionCast(
            PracticalSemanticAnalyzer::StaticType::CPtr sourceType, ExpressionId sourceExpression,
            PracticalSemanticAnalyzer::StaticType::CPtr destType,
            PracticalSemanticAnalyzer::FunctionGen *functionGen);

ValueRangeBase::CPtr unsignedReductionVrp(
            const StaticTypeImpl *sourceType,
            const StaticTypeImpl *destType,
            ValueRangeBase::CPtr inputRange,
            bool isImplicit
        );

ValueRangeBase::CPtr signedReductionVrp(
            const StaticTypeImpl *sourceType,
            const StaticTypeImpl *destType,
            ValueRangeBase::CPtr inputRange,
            bool isImplicit
        );

ValueRangeBase::CPtr signed2UnsignedVrp(
            const StaticTypeImpl *sourceType,
            const StaticTypeImpl *destType,
            ValueRangeBase::CPtr inputRange,
            bool isImplicit
        );

ValueRangeBase::CPtr unsigned2SignedVrp(
            const StaticTypeImpl *sourceType,
            const StaticTypeImpl *destType,
            ValueRangeBase::CPtr inputRange,
            bool isImplicit
        );

ExpressionId changeSignCast(
            PracticalSemanticAnalyzer::StaticType::CPtr sourceType, ExpressionId sourceExpression,
            PracticalSemanticAnalyzer::StaticType::CPtr destType,
            PracticalSemanticAnalyzer::FunctionGen *functionGen);

} // namespace AST

#endif // AST_CASTS_H
