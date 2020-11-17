/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "base.h"

#include <practical/errors.h>

namespace AST::ExpressionImpl {

ExpressionId Base::allocateId() {
    static ExpressionId::Allocator<> allocator;

    return allocator.allocate();
}

Base::~Base() {}

StaticTypeImpl::CPtr Base::getType() const {
    if( castChain )
        return castChain->getMetadata().type;

    return metadata.type;
}

void Base::buildAST( LookupContext &lookupContext, ExpectedResult expectedResult, Weight &weight, Weight weightLimit )
{
    buildASTImpl( lookupContext, expectedResult, weight, weightLimit );

    if( weight>weightLimit )
        throw ExpressionTooExpensive();

    /*
    ASSERT( metadata.type )<<"Build AST did not set a return type "<<getLine()<<":"<<getCol();
    ASSERT( metadata.valueRange )<<"Build AST did not set a value range "<<getLine()<<":"<<getCol();
    */
    if( !expectedResult || *expectedResult.getType()==*metadata.type )
        return;

    castChain = CastChain::allocate(
            lookupContext, expectedResult.getType(), metadata, weight, weightLimit, true, getLine(), getCol() );

    if( !castChain && expectedResult.isMandatory() ) {
        throw CastNotAllowed( metadata.type, expectedResult.getType(), true, getLine(), getCol() );
    }
}

ExpressionId Base::codeGen( PracticalSemanticAnalyzer::FunctionGen *functionGen ) const {
    ExpressionId ret = codeGenImpl( functionGen );

    if( castChain )
        ret = castChain->codeGen( metadata.type, ret, functionGen );

    return ret;
}

} // namespace AST::ExpressionImpl
