/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/casts.h"

#include "ast/expression.h"

namespace AST {

ExpressionId signedExpansionCast( 
            PracticalSemanticAnalyzer::StaticType::CPtr sourceType, ExpressionId sourceExpression,
            PracticalSemanticAnalyzer::StaticType::CPtr destType,
            PracticalSemanticAnalyzer::FunctionGen *functionGen)
{
    ExpressionId id = Expression::allocateId();
    functionGen->expandIntegerSigned( id, sourceExpression, sourceType, destType );

    return id;
}

ExpressionId unsignedExpansionCast( 
            PracticalSemanticAnalyzer::StaticType::CPtr sourceType, ExpressionId sourceExpression,
            PracticalSemanticAnalyzer::StaticType::CPtr destType,
            PracticalSemanticAnalyzer::FunctionGen *functionGen)
{
    ExpressionId id = Expression::allocateId();
    functionGen->expandIntegerUnsigned( id, sourceExpression, sourceType, destType );

    return id;
}

} // namespace AST
