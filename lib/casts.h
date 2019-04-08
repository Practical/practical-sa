/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef CASTS_H
#define CASTS_H

#include "expected_type.h"
#include "practical-sa.h"

namespace Tokenizer {
    struct Token;
}

using PracticalSemanticAnalyzer::StaticType;
using PracticalSemanticAnalyzer::ExpressionId;

bool checkImplicitCastAllowed(
        ExpressionId id, StaticType::Ptr sourceType, ExpectedType destType, const Tokenizer::Token &expressionSource);

ExpressionId codeGenCast(
        PracticalSemanticAnalyzer::FunctionGen *codeGen, ExpressionId sourceExpression, StaticType::Ptr sourceType,
        ExpectedType destType, const Tokenizer::Token &expressionSource, bool implicitOnly );

ExpressionId codeGenCast(
        PracticalSemanticAnalyzer::FunctionGen *codeGen, ExpressionId sourceExpression, StaticType::Ptr sourceType,
        StaticType::Ptr destType, const Tokenizer::Token &expressionSource, bool implicitOnly );

#endif // CASTS_H
