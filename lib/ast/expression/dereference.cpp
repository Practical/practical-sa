/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/expression/dereference.h"

#include "ast/pointers.h"
#include "asserts.h"

#include <practical/errors.h>

namespace AST::ExpressionImpl {

Dereference::Dereference( const NonTerminals::Expression &parserOperand ) :
    operand( parserOperand )
{}

void Dereference::buildASTImpl(
        LookupContext &lookupContext, ExpectedResult expectedResult, ExpressionMetadata &metadata,
        Weight &weight, Weight weightLimit
    )
{
    ExpectedResult expectedOperandResult;

    if( expectedResult ) {
        StaticTypeImpl::CPtr operandExpectedType =
                StaticTypeImpl::allocate( PointerTypeImpl( expectedResult.getType() ) );

        expectedOperandResult = ExpectedResult( std::move(operandExpectedType), expectedResult.isMandatory() );
    }

    operand.buildAST( lookupContext, expectedOperandResult, weight, weightLimit );

    StaticType::Types resultTypeType = operand.getType()->getType();
    auto resultPointerParent = std::get_if<const StaticType::Pointer *>( &resultTypeType );
    if( !resultPointerParent )
        throw PointerExpected(operand.getType(), operand.getLocation());

    const PointerTypeImpl *resultPointer = downCast(*resultPointerParent);
    ASSERT( resultPointer );

    metadata.type = downCast( resultPointer->getPointedType()->setFlags( StaticType::Flags::Reference ) );

    auto operandRange = operand.getValueRange()->downCast< PointerValueRange >();
    if( operandRange->initialized.trueAllowed == false ) {
        throw KnownRuntimeViolation( "Dereferencing a pointer known to be null", operand.getLocation() );
    }
    metadata.valueRange = operandRange->pointedValueRange;
}

ExpressionId Dereference::codeGen( PracticalSemanticAnalyzer::FunctionGen *functionGen ) const {
    return operand.codeGen( functionGen );
}

} // namespace AST::ExpressionImpl
