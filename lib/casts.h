#ifndef CASTS_H
#define CASTS_H

#include "practical-sa.h"

namespace Tokenizer {
    class Token;
}

using PracticalSemanticAnalyzer::StaticType;
using PracticalSemanticAnalyzer::ExpressionId;

void checkImplicitCastAllowed(
        ExpressionId id, const StaticType &sourceType, const StaticType &destType, const Tokenizer::Token &expressionSource);

ExpressionId codeGenCast(
        PracticalSemanticAnalyzer::FunctionGen *codeGen, ExpressionId sourceExpression, const StaticType &sourceType,
        const StaticType &destType, const Tokenizer::Token &expressionSource, bool implicitOnly );

#endif // CASTS_H
