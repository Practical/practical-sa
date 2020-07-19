/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/operators/plus.h"

#include "ast/operators/helper.h"

namespace AST::Operators {

using namespace PracticalSemanticAnalyzer;

ExpressionId bPlusCodegen(
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
    functionGen->binaryOperatorPlus(
            resultId, argumentIds[0], argumentIds[1],
            std::get<const StaticType::Function *>(definition->type->getType())->getReturnType() );

    return resultId;
}

ValueRangeBase::CPtr bPlusUnsignedVrp(StaticTypeImpl::CPtr funcType, Slice<ValueRangeBase::CPtr> inputRangesBase)
{
    auto inputRanges = downcastValueRanges<UnsignedIntValueRange>( inputRangesBase );
    ABORT()<<"TODO implement";
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
        // UB can't rule out signed underflow
        /* There is no underflow iff: in[0]+in[1] >= range.min
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
        // UB can't rule out signed underflow
        ret->minimum = typeRange->minimum;
    } else {
        ret->minimum = inputRanges[0]->minimum + inputRanges[1]->minimum;
    }

    return ret;
}

} // namespace AST::Operators
