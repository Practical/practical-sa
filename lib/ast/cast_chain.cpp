/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/cast_chain.h"

namespace AST {

std::unique_ptr<CastChain> CastChain::allocate(
        const LookupContext &lookupContext,
        StaticTypeImpl::CPtr destinationType,
        const ExpressionImpl::ExpressionMetadata &srcMetadata,
        unsigned &weight,
        unsigned weightLimit )
{
}

ExpressionId CastChain::codeGen(
            PracticalSemanticAnalyzer::StaticType::CPtr sourceType, ExpressionId sourceExpression,
            PracticalSemanticAnalyzer::FunctionGen *functionGen
        ) const
{
    ExpressionId previousResult = sourceExpression;
    if( previousCast ) {
        previousResult = previousCast->codeGen( sourceType, sourceExpression, functionGen );
        sourceType = previousCast->getMetadata().type;
    }

    return cast.codeGen( sourceType, previousResult, metadata.type, functionGen );
}

} // namespace AST
