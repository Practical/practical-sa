/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_CAST_OP_H
#define AST_CAST_OP_H

#include "ast/lookup_context.h"
#include "ast/static_type.h"

namespace AST {

class CastOperation : private NoCopy {
    LookupContext::CodeGenCast castFunction;
    StaticType::CPtr sourceType, destType;

public:
    explicit CastOperation( LookupContext::CodeGenCast castFunction, StaticType::CPtr sourceType, StaticType::CPtr destType );

    StaticType::CPtr getType() const {
        return destType;
    }

    PracticalSemanticAnalyzer::ExpressionId codeGen(
            PracticalSemanticAnalyzer::ExpressionId source,
            PracticalSemanticAnalyzer::FunctionGen *functionGen);
};

} // namespace AST

#endif // AST_CAST_OP_H
