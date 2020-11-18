/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/expression/address_of.h"

#include "ast/pointers.h"

#include <practical/errors.h>
#include <practical/practical.h>

using namespace PracticalSemanticAnalyzer;

namespace AST::ExpressionImpl {

using namespace PracticalSemanticAnalyzer;

AddressOf::AddressOf( const NonTerminals::Expression &parserOperand ) :
    operand( parserOperand )
{}

void AddressOf::buildASTImpl(
        LookupContext &lookupContext, ExpectedResult expectedResult, ExpressionMetadata &metadata,
        Weight &weight, Weight weightLimit
    )
{
    StaticType::Types expectedType = expectedResult.getType()->getType();
    auto expectedPointer = std::get_if<const StaticType::Pointer *>( &expectedType );

    if( expectedPointer ) {
        operand.buildAST(
                lookupContext,
                ExpectedResult(
                    (*expectedPointer)->getPointedType()->addFlags( StaticType::Flags::Reference ),
                    expectedResult.isMandatory() ),
                weight,
                weightLimit
            );
    } else {
        operand.buildAST( lookupContext, ExpectedResult(), weight, weightLimit );
    }

    if( (operand.getType()->getFlags() & StaticType::Flags::Reference)==0 )
        throw LValueRequired( operand.getType(), operand.getLocation() );

    metadata.type = StaticTypeImpl::allocate( PointerTypeImpl( operand.getType() ) );
    metadata.valueRange = new PointerValueRange( operand.getValueRange() );
}

ExpressionId AddressOf::codeGen( PracticalSemanticAnalyzer::FunctionGen *functionGen ) const {
    ASSERT( (operand.getType()->getFlags() & StaticType::Flags::Reference) != 0 )<<
            "Address of operand type "<<operand.getType()<<" is not an lvalue";

    return operand.codeGen(functionGen);
}

} // AST::ExpressionImpl
