/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/operators/boolean.h"

#include "ast/operators/helper.h"
#include "ast/ast.h"
#include "ast/bool_value_range.h"
#include "ast/unsigned_int_value_range.h"

namespace AST::Operators {

using namespace PracticalSemanticAnalyzer;

template<
    void (FunctionGen::*generator)(
                ExpressionId, ExpressionId, ExpressionId, StaticType::CPtr )
        >
static ExpressionId genericCodeGen(
        Slice<const Expression> arguments,
        const LookupContext::Function::Definition *definition,
        FunctionGen *functionGen)
{
    ASSERT(arguments.size()==2);

    std::array<ExpressionId,2> argumentIds;
    for( unsigned i=0; i<2; ++i ) {
        argumentIds[i] = arguments[i].codeGen(functionGen);
    }

    ExpressionId resultId = ExpressionImpl::Base::allocateId();
    (functionGen->*generator)(
            resultId, argumentIds[0], argumentIds[1],
            std::get<const StaticType::Function *>(definition->type->getType())->getReturnType() );

    return resultId;
}

ExpressionId equalsCodegenInt(
        Slice<const Expression> arguments,
        const LookupContext::Function::Definition *definition,
        PracticalSemanticAnalyzer::FunctionGen *functionGen)
{
    return genericCodeGen< &FunctionGen::operatorEquals >(arguments, definition, functionGen);
}

template<typename VR, bool negate>
static ValueRangeBase::CPtr equalsVrpImpl(StaticTypeImpl::CPtr functType, Slice<ValueRangeBase::CPtr> inputRangesBase)
{
    ASSERT( inputRangesBase.size()==2 );
    auto inputRanges = downcastValueRanges<VR>( inputRangesBase );

    // True is not an option iff there is no intersection between the ranges
    if( inputRanges[0]->maximum<inputRanges[1]->minimum || inputRanges[0]->minimum > inputRanges[1]->maximum ) {
        if constexpr( !negate )
            return BoolValueRange::allocate( false, true );
        else
            return BoolValueRange::allocate( true, false );
    }

    // False is not an option iff both are literals and identical
    if(
            inputRanges[0]->isLiteral() && inputRanges[1]->isLiteral() &&
            inputRanges[0]->minimum==inputRanges[1]->minimum
      ) {
        if constexpr( !negate )
            return BoolValueRange::allocate( true, false );
        else
            return BoolValueRange::allocate( false, true );
    }

    return BoolValueRange::allocate( true, true );
}

ValueRangeBase::CPtr equalsVrpUnsigned(StaticTypeImpl::CPtr functType, Slice<ValueRangeBase::CPtr> inputRangesBase) {
    return equalsVrpImpl<UnsignedIntValueRange, false>( std::move(functType), inputRangesBase );
}

ValueRangeBase::CPtr equalsVrpSigned(StaticTypeImpl::CPtr functType, Slice<ValueRangeBase::CPtr> inputRangesBase) {
    return equalsVrpImpl<SignedIntValueRange, false>( std::move(functType), inputRangesBase );
}

ExpressionId notEqualsCodegenInt(
        Slice<const Expression> arguments,
        const LookupContext::Function::Definition *definition,
        PracticalSemanticAnalyzer::FunctionGen *functionGen)
{
    return genericCodeGen< &FunctionGen::operatorNotEquals >(arguments, definition, functionGen);
}

ValueRangeBase::CPtr notEqualsVrpUnsigned(StaticTypeImpl::CPtr functType, Slice<ValueRangeBase::CPtr> inputRangesBase) {
    return equalsVrpImpl<UnsignedIntValueRange, true>( std::move(functType), inputRangesBase );
}

ValueRangeBase::CPtr notEqualsVrpSigned(StaticTypeImpl::CPtr functType, Slice<ValueRangeBase::CPtr> inputRangesBase) {
    return equalsVrpImpl<SignedIntValueRange, true>( std::move(functType), inputRangesBase );
}


template<typename VR, bool Equals, bool Negate>
static ValueRangeBase::CPtr lessThanVrpImpl(StaticTypeImpl::CPtr functType, Slice<ValueRangeBase::CPtr> inputRangesBase)
{
    auto inputRanges = downcastValueRanges<VR>( inputRangesBase );
    ASSERT( inputRangesBase.size()==2 );

    // Cannot be True iff left is entirely above right
    if(
            inputRanges[0]->minimum >= inputRanges[1]->maximum && (
                !Equals ||
                inputRanges[0]->minimum != inputRanges[1]->maximum
            )
      )
    {
        if constexpr( !Negate )
            return BoolValueRange::allocate( true, false );
        else
            return BoolValueRange::allocate( false, true );
    }

    // Cannot be False iff left is entirely below right
    if(
            inputRanges[0]->maximum <= inputRanges[1]->minimum && (
                Equals ||
                inputRanges[0]->maximum != inputRanges[1]->minimum )
      )
    {
        if constexpr( !Negate )
            return BoolValueRange::allocate( false, true );
        else
            return BoolValueRange::allocate( true, false );
    }

    return BoolValueRange::allocate( true, true );
}

ExpressionId lessThanCodegenUInt(
        Slice<const Expression> arguments,
        const LookupContext::Function::Definition *definition,
        PracticalSemanticAnalyzer::FunctionGen *functionGen)
{
    return genericCodeGen< &FunctionGen::operatorLessThanUnsigned >(arguments, definition, functionGen);
}

ValueRangeBase::CPtr lessThanVrpUnsigned(StaticTypeImpl::CPtr funcType, Slice<ValueRangeBase::CPtr> inputRanges)
{
    return lessThanVrpImpl< UnsignedIntValueRange, false, false >( std::move(funcType), inputRanges );
}

ExpressionId lessThanCodegenSInt(
        Slice<const Expression> arguments,
        const LookupContext::Function::Definition *definition,
        PracticalSemanticAnalyzer::FunctionGen *functionGen)
{
    return genericCodeGen< &FunctionGen::operatorLessThanSigned >(arguments, definition, functionGen);
}

ValueRangeBase::CPtr lessThanVrpSigned(StaticTypeImpl::CPtr funcType, Slice<ValueRangeBase::CPtr> inputRanges)
{
    return lessThanVrpImpl< SignedIntValueRange, false, false >( std::move(funcType), inputRanges );
}


ExpressionId lessThanOrEqualsCodegenUInt(
        Slice<const Expression> arguments,
        const LookupContext::Function::Definition *definition,
        PracticalSemanticAnalyzer::FunctionGen *functionGen)
{
    return genericCodeGen< &FunctionGen::operatorLessThanOrEqualsUnsigned >(arguments, definition, functionGen);
}

ValueRangeBase::CPtr lessThanOrEqualsVrpUnsigned(StaticTypeImpl::CPtr funcType, Slice<ValueRangeBase::CPtr> inputRanges)
{
    return lessThanVrpImpl< UnsignedIntValueRange, true, false >( std::move(funcType), inputRanges );
}

ExpressionId lessThanOrEqualsCodegenSInt(
        Slice<const Expression> arguments,
        const LookupContext::Function::Definition *definition,
        PracticalSemanticAnalyzer::FunctionGen *functionGen)
{
    return genericCodeGen< &FunctionGen::operatorLessThanOrEqualsSigned >(arguments, definition, functionGen);
}

ValueRangeBase::CPtr lessThanOrEqualsVrpSigned(StaticTypeImpl::CPtr funcType, Slice<ValueRangeBase::CPtr> inputRanges)
{
    return lessThanVrpImpl< SignedIntValueRange, true, false >( std::move(funcType), inputRanges );
}


ExpressionId greaterThenCodegenUInt(
        Slice<const Expression> arguments,
        const LookupContext::Function::Definition *definition,
        PracticalSemanticAnalyzer::FunctionGen *functionGen)
{
    return genericCodeGen< &FunctionGen::operatorGreaterThanUnsigned >(arguments, definition, functionGen);
}

ValueRangeBase::CPtr greaterThenVrpUnsigned(StaticTypeImpl::CPtr funcType, Slice<ValueRangeBase::CPtr> inputRanges)
{
    return lessThanVrpImpl< UnsignedIntValueRange, true, true >( std::move(funcType), inputRanges );
}

ExpressionId greaterThenCodegenSInt(
        Slice<const Expression> arguments,
        const LookupContext::Function::Definition *definition,
        PracticalSemanticAnalyzer::FunctionGen *functionGen)
{
    return genericCodeGen< &FunctionGen::operatorGreaterThanSigned >(arguments, definition, functionGen);
}

ValueRangeBase::CPtr greaterThenVrpSigned(StaticTypeImpl::CPtr funcType, Slice<ValueRangeBase::CPtr> inputRanges)
{
    return lessThanVrpImpl< SignedIntValueRange, true, true >( std::move(funcType), inputRanges );
}


ExpressionId greaterThenOrEqualsCodegenUInt(
        Slice<const Expression> arguments,
        const LookupContext::Function::Definition *definition,
        PracticalSemanticAnalyzer::FunctionGen *functionGen)
{
    return genericCodeGen< &FunctionGen::operatorGreaterThanOrEqualsUnsigned >(arguments, definition, functionGen);
}

ValueRangeBase::CPtr greaterThenOrEqualsVrpUnsigned(StaticTypeImpl::CPtr funcType, Slice<ValueRangeBase::CPtr> inputRanges)
{
    return lessThanVrpImpl< UnsignedIntValueRange, false, true >( std::move(funcType), inputRanges );
}

ExpressionId greaterThenOrEqualsCodegenSInt(
        Slice<const Expression> arguments,
        const LookupContext::Function::Definition *definition,
        PracticalSemanticAnalyzer::FunctionGen *functionGen)
{
    return genericCodeGen< &FunctionGen::operatorGreaterThanOrEqualsSigned >(arguments, definition, functionGen);
}

ValueRangeBase::CPtr greaterThenOrEqualsVrpSigned(StaticTypeImpl::CPtr funcType, Slice<ValueRangeBase::CPtr> inputRanges)
{
    return lessThanVrpImpl< SignedIntValueRange, false, true >( std::move(funcType), inputRanges );
}


ExpressionId logicalAnd(
        Slice<const Expression> arguments,
        const LookupContext::Function::Definition *definition,
        PracticalSemanticAnalyzer::FunctionGen *functionGen)
{
    ASSERT(arguments.size()==2);

    /* We are effectively codegening the following code:
     * if( left ) {
     *   right
     * } else {
     *   false
     * }
     */

    ExpressionId leftArgumentId = arguments[0].codeGen(functionGen);
    ExpressionId resultId = ExpressionImpl::Base::allocateId();

    JumpPointId elsePoint = jumpPointAllocator.allocate(),
                contPoint = jumpPointAllocator.allocate();

    functionGen->conditionalBranch( resultId, definition->returnType(), leftArgumentId, elsePoint, contPoint );

    // Then clause - left side was true
    ExpressionId rightArgumentId = arguments[1].codeGen(functionGen);
    // This is the expression's result
    functionGen->setConditionClauseResult( rightArgumentId );

    functionGen->setJumpPoint( elsePoint );
    // Else clause - left side was false
    ExpressionId literalFalse = ExpressionImpl::Base::allocateId();
    functionGen->setLiteral( literalFalse, false );
    functionGen->setConditionClauseResult( literalFalse );

    // Continuation
    functionGen->setJumpPoint( contPoint );

    return resultId;
}

ValueRangeBase::CPtr logicalAndVrp(
        StaticTypeImpl::CPtr functType, Slice<ValueRangeBase::CPtr> inputRangesBase)
{
    ASSERT( inputRangesBase.size()==2 );
    auto inputRanges = downcastValueRanges<BoolValueRange>( inputRangesBase );

    return new BoolValueRange(
            inputRanges[0]->falseAllowed || inputRanges[1]->falseAllowed,
            inputRanges[0]->trueAllowed && inputRanges[1]->trueAllowed );
}


ExpressionId logicalOr(
        Slice<const Expression> arguments,
        const LookupContext::Function::Definition *definition,
        PracticalSemanticAnalyzer::FunctionGen *functionGen)
{
    ASSERT(arguments.size()==2);

    /* We are effectively codegening the following code:
     * if( left ) {
     *   true
     * } else {
     *   right
     * }
     */

    ExpressionId leftArgumentId = arguments[0].codeGen(functionGen);
    ExpressionId resultId = ExpressionImpl::Base::allocateId();

    JumpPointId elsePoint = jumpPointAllocator.allocate(),
                contPoint = jumpPointAllocator.allocate();

    functionGen->conditionalBranch( resultId, definition->returnType(), leftArgumentId, elsePoint, contPoint );

    // Then clause - left side was true
    ExpressionId literalTrue = ExpressionImpl::Base::allocateId();
    functionGen->setLiteral( literalTrue, true );
    // This is the expression's result
    functionGen->setConditionClauseResult( literalTrue );


    functionGen->setJumpPoint( elsePoint );
    // Else clause - left side was false
    ExpressionId rightArgumentId = arguments[1].codeGen(functionGen);

    functionGen->setConditionClauseResult( rightArgumentId );

    // Continuation
    functionGen->setJumpPoint( contPoint );

    return resultId;
}

ValueRangeBase::CPtr logicalOrVrp(StaticTypeImpl::CPtr functType, Slice<ValueRangeBase::CPtr> inputRangesBase)
{
    ASSERT( inputRangesBase.size()==2 );
    auto inputRanges = downcastValueRanges<BoolValueRange>( inputRangesBase );

    return new BoolValueRange(
            inputRanges[0]->falseAllowed && inputRanges[1]->falseAllowed,
            inputRanges[0]->trueAllowed || inputRanges[1]->trueAllowed );
}

} // namespace AST::Operators
