/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "base.h"

namespace AST::ExpressionImpl {

ExpressionId Base::allocateId() {
    static ExpressionId::Allocator<> allocator;

    return allocator.allocate();
}

Base::~Base() {}

PracticalSemanticAnalyzer::StaticType::CPtr Base::getType() const {
    if( castOp )
        return castOp->getType();

    return metadata.type;
}

void Base::buildAST( LookupContext &lookupContext, ExpectedResult expectedResult, unsigned &weight, unsigned weightLimit )
{
    buildASTImpl( lookupContext, expectedResult, weight, weightLimit );

    if( !expectedResult || *expectedResult.getType()==*metadata.type )
        return;

    auto castFunction = lookupContext.lookupCast( metadata.type, expectedResult.getType(), true );
    if( castFunction ) {
        castOp = safenew<CastOperation>( castFunction, metadata.type, expectedResult.getType() );

        return;
    }

    if( expectedResult.isMandatory() ) {
        ABORT()<<"TODO implement multi-step casts";
    }
}

ExpressionId Base::codeGen( PracticalSemanticAnalyzer::FunctionGen *functionGen ) {
    ExpressionId ret = codeGenImpl( functionGen );

    if( castOp )
        ret = castOp->codeGen( ret, functionGen );

    return ret;
}

} // namespace AST::ExpressionImpl
