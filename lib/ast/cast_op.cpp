/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/cast_op.h"

namespace AST {

CastOperation::CastOperation(
        LookupContext::CodeGenCast castFunction, StaticType::CPtr sourceType, StaticType::CPtr destType ) :
    castFunction(castFunction),
    sourceType(sourceType),
    destType(destType)
{
}

PracticalSemanticAnalyzer::ExpressionId CastOperation::codeGen(
        PracticalSemanticAnalyzer::ExpressionId source,
        PracticalSemanticAnalyzer::FunctionGen *functionGen)
{
    return castFunction(sourceType, source, destType, functionGen);
}

} // namespace AST
