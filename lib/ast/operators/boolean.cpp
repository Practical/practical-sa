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
#include "ast/bool_value_range.h"
#include "ast/unsigned_int_value_range.h"

namespace AST::Operators {

ExpressionId equalsCodegenInt(
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
    functionGen->operatorEquals(
            resultId, argumentIds[0], argumentIds[1],
            std::get<const StaticType::Function *>(definition->type->getType())->getReturnType() );

    return resultId;
}

template<typename VR>
static ValueRangeBase::CPtr equalsVrpImpl(StaticTypeImpl::CPtr functType, Slice<ValueRangeBase::CPtr> inputRangesBase)
{
    auto inputRanges = downcastValueRanges<VR>( inputRangesBase );
    ASSERT( inputRangesBase.size()==2 );

    // True is not an option iff there is no intersection between the ranges
    if( inputRanges[0]->maximum<inputRanges[1]->minimum || inputRanges[0]->minimum > inputRanges[1]->maximum )
        return BoolValueRange::allocate( false, true );

    // False is not an option iff both are literals and identical
    if(
            inputRanges[0]->isLiteral() && inputRanges[1]->isLiteral() &&
            inputRanges[0]->minimum==inputRanges[1]->minimum
      )
        return BoolValueRange::allocate( true, false );

    return BoolValueRange::allocate( true, true );
}

ValueRangeBase::CPtr equalsVrpUnsigned(StaticTypeImpl::CPtr functType, Slice<ValueRangeBase::CPtr> inputRangesBase) {
    return equalsVrpImpl<UnsignedIntValueRange>( std::move(functType), inputRangesBase );
}

ValueRangeBase::CPtr equalsVrpSigned(StaticTypeImpl::CPtr functType, Slice<ValueRangeBase::CPtr> inputRangesBase) {
    return equalsVrpImpl<SignedIntValueRange>( std::move(functType), inputRangesBase );
}

} // namespace AST::Operators
