/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/operators/algebraic_int.h"

#include "ast/operators/helper.h"

namespace AST::Operators {

using namespace PracticalSemanticAnalyzer;

// Plus

ExpressionId bPlusCodegenUnsigned(
        Slice<const Expression> arguments,
        const LookupContext::Function::Definition *definition,
        PracticalSemanticAnalyzer::FunctionGen *functionGen)
{
    ASSERT(arguments.size()==2);

    std::array<ExpressionId,2> argumentIds;
    for( unsigned i=0; i<2; ++i ) {
        argumentIds[i] = arguments[i].codeGen(functionGen);
    }

    ExpressionId resultId = ExpressionImpl::Base::allocateId();
    functionGen->binaryOperatorPlusUnsigned(
            resultId, argumentIds[0], argumentIds[1],
            std::get<const StaticType::Function *>(definition->type->getType())->getReturnType() );

    return resultId;
}

ValueRangeBase::CPtr bPlusUnsignedVrp(StaticTypeImpl::CPtr funcType, Slice<ValueRangeBase::CPtr> inputRangesBase)
{
    auto inputRanges = downcastValueRanges<UnsignedIntValueRange>( inputRangesBase );
    ASSERT( inputRangesBase.size()==2 );

    const UnsignedIntValueRange *typeRange = getUnsignedOverloadRange( funcType, inputRanges );
    auto ret = UnsignedIntValueRange::allocate( typeRange );

    bool maxOverflow = false;
    if( inputRanges[0]->maximum>0 && (typeRange->maximum-inputRanges[0]->maximum) < inputRanges[1]->maximum ) {
        /* There is no overflow iff: in[0]+in[1] <= range.max
         * in[1] <= range.max-in[0]
         * Reversal: range.max-in[0] < in[1]
         */
        maxOverflow = true;
    }

    bool minOverflow = false;
    if( inputRanges[0]->minimum>0 && (typeRange->maximum-inputRanges[0]->minimum) < inputRanges[1]->minimum ) {
        ASSERT( maxOverflow );
        minOverflow = true;
    }

    if( minOverflow || !maxOverflow ) {
        ret->minimum = inputRanges[0]->minimum + inputRanges[1]->minimum;
        ret->minimum &= typeRange->maximum;
        ret->maximum = inputRanges[0]->maximum + inputRanges[1]->maximum;
        ret->maximum &= typeRange->maximum;
    } else {
        ret->minimum = typeRange->minimum;
        ret->maximum = typeRange->maximum;
    }

    return ret;
}

ExpressionId bPlusCodegenSigned(
        Slice<const Expression> arguments,
        const LookupContext::Function::Definition *definition,
        PracticalSemanticAnalyzer::FunctionGen *functionGen)
{
    ASSERT(arguments.size()==2);

    std::array<ExpressionId,2> argumentIds;
    for( unsigned i=0; i<2; ++i ) {
        argumentIds[i] = arguments[i].codeGen(functionGen);
    }

    ExpressionId resultId = ExpressionImpl::Base::allocateId();
    functionGen->binaryOperatorPlusSigned(
            resultId, argumentIds[0], argumentIds[1],
            std::get<const StaticType::Function *>(definition->type->getType())->getReturnType() );

    return resultId;
}

ValueRangeBase::CPtr bPlusSignedVrp(StaticTypeImpl::CPtr funcType, Slice<ValueRangeBase::CPtr> inputRangesBase)
{
    auto inputRanges = downcastValueRanges<SignedIntValueRange>( inputRangesBase );
    ASSERT( inputRangesBase.size()==2 );

    const SignedIntValueRange *typeRange = getSignedOverloadRange( funcType, inputRanges );
    auto ret = SignedIntValueRange::allocate( typeRange );

    if( inputRanges[0]->maximum>0 && (typeRange->maximum-inputRanges[0]->maximum) < inputRanges[1]->maximum ) {
        // UB can't rule out signed overflow
        /* There is no overflow iff: in[0]+in[1] <= range.max
         * in[1] <= range.max-in[0]
         * Reversal: range.max-in[0] < in[1]
         */
        ret->maximum = typeRange->maximum;
    } else if( inputRanges[0]->maximum<0 && (typeRange->minimum-inputRanges[0]->maximum) > inputRanges[1]->maximum ) {
        // UB can't rule out signed negative overflow
        /* There is no negative overflow iff: in[0]+in[1] >= range.min
         * in[1] >= range.min-in[0]
         * Reversal of condition: range.min-in[0] > in[1]
         */
        ret->maximum = typeRange->minimum;
    } else {
        ret->maximum = inputRanges[0]->maximum + inputRanges[1]->maximum;
    }

    if( inputRanges[0]->minimum>0 && (typeRange->maximum-inputRanges[0]->minimum) < inputRanges[1]->minimum ) {
        // UB can't rule out signed overflow
        ret->minimum = typeRange->maximum;
        ASSERT( ret->maximum == typeRange->maximum );
    } else if( inputRanges[0]->minimum<0 && (typeRange->minimum-inputRanges[0]->minimum) > inputRanges[1]->minimum ) {
        // UB can't rule out signed negative overflow
        ret->minimum = typeRange->minimum;
    } else {
        ret->minimum = inputRanges[0]->minimum + inputRanges[1]->minimum;
    }

    return ret;
}


// Minus

ExpressionId bMinusCodegenUnsigned(
        Slice<const Expression> arguments,
        const LookupContext::Function::Definition *definition,
        PracticalSemanticAnalyzer::FunctionGen *functionGen)
{
    ASSERT(arguments.size()==2);

    std::array<ExpressionId,2> argumentIds;
    for( unsigned i=0; i<2; ++i ) {
        argumentIds[i] = arguments[i].codeGen(functionGen);
    }

    ExpressionId resultId = ExpressionImpl::Base::allocateId();
    functionGen->binaryOperatorMinusUnsigned(
            resultId, argumentIds[0], argumentIds[1],
            std::get<const StaticType::Function *>(definition->type->getType())->getReturnType() );

    return resultId;
}

ValueRangeBase::CPtr bMinusUnsignedVrp(StaticTypeImpl::CPtr funcType, Slice<ValueRangeBase::CPtr> inputRangesBase)
{
    auto inputRanges = downcastValueRanges<UnsignedIntValueRange>( inputRangesBase );
    ASSERT( inputRangesBase.size()==2 );

    const UnsignedIntValueRange *typeRange = getUnsignedOverloadRange( funcType, inputRanges );
    auto ret = UnsignedIntValueRange::allocate( typeRange );

    bool maxOverflow = false;
    bool minOverflow = false;
    if( inputRanges[0]->maximum < inputRanges[1]->minimum ) {
        // Biggest number is a.max - b.min
        maxOverflow = true;
        minOverflow = true;
    } else if( inputRanges[0]->minimum < inputRanges[1]->maximum ) {
        // Smallest is a.min - b.max
        minOverflow = true;
    }

    if( minOverflow || !maxOverflow ) {
        // Either both or neither overflow. Destination min and max overflowed the same number of times
        ret->minimum = inputRanges[0]->minimum - inputRanges[1]->maximum;
        ret->minimum &= typeRange->maximum;
        ret->maximum = inputRanges[0]->maximum - inputRanges[1]->minimum;
        ret->maximum &= typeRange->maximum;
    } else {
        ret->minimum = typeRange->minimum;
        ret->maximum = typeRange->maximum;
    }

    return ret;
}

ExpressionId bMinusCodegenSigned(
        Slice<const Expression> arguments,
        const LookupContext::Function::Definition *definition,
        PracticalSemanticAnalyzer::FunctionGen *functionGen)
{
    ASSERT(arguments.size()==2);

    std::array<ExpressionId,2> argumentIds;
    for( unsigned i=0; i<2; ++i ) {
        argumentIds[i] = arguments[i].codeGen(functionGen);
    }

    ExpressionId resultId = ExpressionImpl::Base::allocateId();
    functionGen->binaryOperatorMinusSigned(
            resultId, argumentIds[0], argumentIds[1],
            std::get<const StaticType::Function *>(definition->type->getType())->getReturnType() );

    return resultId;
}

ValueRangeBase::CPtr bMinusSignedVrp(StaticTypeImpl::CPtr funcType, Slice<ValueRangeBase::CPtr> inputRangesBase)
{
    auto inputRanges = downcastValueRanges<SignedIntValueRange>( inputRangesBase );
    ASSERT( inputRangesBase.size()==2 );

    const SignedIntValueRange *typeRange = getSignedOverloadRange( funcType, inputRanges );
    auto ret = SignedIntValueRange::allocate( typeRange );

    if( inputRanges[1]->minimum>=0 || inputRanges[0]->maximum <= typeRange->maximum + inputRanges[1]->minimum )
    {
        /* Won't overflow iff:
         * in[1].min>0
         * or
         * in[0].max - in[1].min <= range.max
         * in[0].max <= range.max + in[1].min
         */

        // Make sure the maximal result doesn't negative overflow
        if( inputRanges[1]->minimum<0 || inputRanges[0]->maximum >= typeRange->minimum + inputRanges[1]->minimum )
        {
            /* Won't overflow iff:
             * in[1].min<0
             * or
             * in[0].max - in[1].min >= range.min
             * in[0].max >= range.min + in[1].min
             */
            ret->maximum = inputRanges[0]->maximum - inputRanges[1]->minimum;
        } else {
            // The maximal result performs a negative overflow. UB is guaranteed.
            // XXX no context for exception
            //throw UndefinedBehavior( "Guaranteed signed integer negative overflow", 0, 0 );
            ret->maximum = ret->minimum = typeRange->minimum;
            return ret;
        }
    } else {
        // UB can't rule out signed overflow.
        ret->maximum = typeRange->maximum;
    }

    if( inputRanges[1]->maximum<0 || inputRanges[0]->minimum >= typeRange->minimum + inputRanges[1]->maximum )
    {
        /* Won't negative overflow iff:
         * in[1].max<0
         * or
         * in[0].min - in[1].max >= range.min
         * in[0].min >= range.min + in[1].max
         */

        // Make sure the minimal result doesn't positive overflow
        if( inputRanges[1]->maximum>0 || inputRanges[0]->minimum <= typeRange->maximum - inputRanges[1]->maximum )
        {
            /* Won't overflow iff:
             * in[1].max>0
             * or
             * in[0].min - in[1].max <= range.max
             * in[0].min <= range.max + in[1].max
             */
            ret->minimum = inputRanges[0]->minimum - inputRanges[1]->maximum;
        } else {
            // The maximal result performs a negative overflow. UB is guaranteed.
            // XXX no context for exception
            //throw UndefinedBehavior( "Guaranteed signed integer positive overflow", 0, 0 );
            ret->maximum = ret->minimum = typeRange->maximum;
            return ret;
        }
    } else {
        // UB can't rule out signed negative overflow.
        ret->minimum = typeRange->minimum;
    }

    return ret;
}


// Multiply

ExpressionId bMultiplyCodegenUnsigned(
        Slice<const Expression> arguments,
        const LookupContext::Function::Definition *definition,
        PracticalSemanticAnalyzer::FunctionGen *functionGen)
{
    ASSERT(arguments.size()==2);

    std::array<ExpressionId,2> argumentIds;
    for( unsigned i=0; i<2; ++i ) {
        argumentIds[i] = arguments[i].codeGen(functionGen);
    }

    ExpressionId resultId = ExpressionImpl::Base::allocateId();
    functionGen->binaryOperatorMultiplyUnsigned(
            resultId, argumentIds[0], argumentIds[1],
            std::get<const StaticType::Function *>(definition->type->getType())->getReturnType() );

    return resultId;
}

ValueRangeBase::CPtr bMultiplyUnsignedVrp(StaticTypeImpl::CPtr funcType, Slice<ValueRangeBase::CPtr> inputRangesBase)
{
    auto inputRanges = downcastValueRanges<UnsignedIntValueRange>( inputRangesBase );
    ASSERT( inputRangesBase.size()==2 );

    const UnsignedIntValueRange *typeRange = getUnsignedOverloadRange( funcType, inputRanges );
    auto ret = UnsignedIntValueRange::allocate( typeRange );

    LongEnoughInt tmpResult;
    bool overflowed = __builtin_mul_overflow( inputRanges[0]->maximum, inputRanges[1]->maximum, &tmpResult );
    ret->maximum = tmpResult & typeRange->maximum;
    if( overflowed || ret->maximum!=tmpResult ) {
        // We overflowed
        return typeRange;
    }

    overflowed = __builtin_mul_overflow( inputRanges[0]->minimum, inputRanges[1]->minimum, &ret->minimum );
    ASSERT( !overflowed && ret->minimum == (ret->minimum & typeRange->maximum) )<<
            "Unsigned multiplication minimal value overflowed while maximal did not";

    return ret;
}

ExpressionId bMultiplyCodegenSigned(
        Slice<const Expression> arguments,
        const LookupContext::Function::Definition *definition,
        PracticalSemanticAnalyzer::FunctionGen *functionGen)
{
    ASSERT(arguments.size()==2);

    std::array<ExpressionId,2> argumentIds;
    for( unsigned i=0; i<2; ++i ) {
        argumentIds[i] = arguments[i].codeGen(functionGen);
    }

    ExpressionId resultId = ExpressionImpl::Base::allocateId();
    functionGen->binaryOperatorMultiplySigned(
            resultId, argumentIds[0], argumentIds[1],
            std::get<const StaticType::Function *>(definition->type->getType())->getReturnType() );

    return resultId;
}

ValueRangeBase::CPtr bMultiplySignedVrp(StaticTypeImpl::CPtr funcType, Slice<ValueRangeBase::CPtr> inputRangesBase)
{
    auto inputRanges = downcastValueRanges<SignedIntValueRange>( inputRangesBase );
    ASSERT( inputRangesBase.size()==2 );

    const SignedIntValueRange *typeRange = getSignedOverloadRange( funcType, inputRanges );

    LongEnoughIntSigned tmp1, tmp2, tmp3, tmp4;

    // XXX We depend on 2's complement representation
#define CHECK_OVERFLOW(a) ( (a)>0 ? ( ((a)&typeRange->maximum)!=(a)) : ( ((a)&typeRange->minimum) != typeRange->minimum) )
    bool overflowed = __builtin_mul_overflow( inputRanges[0]->maximum, inputRanges[1]->maximum, &tmp1 ) ||
            CHECK_OVERFLOW(tmp1);
    overflowed = overflowed || __builtin_mul_overflow( inputRanges[0]->maximum, inputRanges[1]->minimum, &tmp2 ) ||
            CHECK_OVERFLOW(tmp2);
    overflowed = overflowed || __builtin_mul_overflow( inputRanges[0]->minimum, inputRanges[1]->maximum, &tmp3 ) ||
            CHECK_OVERFLOW(tmp3);
    overflowed = overflowed || __builtin_mul_overflow( inputRanges[0]->minimum, inputRanges[1]->minimum, &tmp4 ) ||
            CHECK_OVERFLOW(tmp4);
#undef CHECK_OVERFLOW

    if( overflowed ) {
        return typeRange;
    }

    return SignedIntValueRange::allocate(
            std::min( std::min(tmp1, tmp2), std::min(tmp3, tmp4) ),
            std::max( std::max(tmp1, tmp2), std::max(tmp3, tmp4) )
        );
}

// Divide
ExpressionId bDivideCodegenUnsigned(
        Slice<const Expression> arguments,
        const LookupContext::Function::Definition *definition,
        PracticalSemanticAnalyzer::FunctionGen *functionGen)
{
    ASSERT(arguments.size()==2);

    std::array<ExpressionId,2> argumentIds;
    for( unsigned i=0; i<2; ++i ) {
        argumentIds[i] = arguments[i].codeGen(functionGen);
    }

    ExpressionId resultId = ExpressionImpl::Base::allocateId();
    functionGen->binaryOperatorDivideUnsigned(
            resultId, argumentIds[0], argumentIds[1],
            std::get<const StaticType::Function *>(definition->type->getType())->getReturnType() );

    return resultId;
}

ValueRangeBase::CPtr bDivideUnsignedVrp(StaticTypeImpl::CPtr funcType, Slice<ValueRangeBase::CPtr> inputRangesBase)
{
    auto inputRanges = downcastValueRanges<UnsignedIntValueRange>( inputRangesBase );
    ASSERT( inputRangesBase.size()==2 );

    const UnsignedIntValueRange *typeRange = getUnsignedOverloadRange( funcType, inputRanges );
    auto ret = UnsignedIntValueRange::allocate( typeRange );

    ret->maximum = inputRanges[0]->maximum / inputRanges[1]->minimum;
    ret->minimum = inputRanges[0]->minimum / inputRanges[1]->maximum;

    return ret;
}


} // namespace AST::Operators
