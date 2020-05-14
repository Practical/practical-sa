/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_EXPRESSION_FUNCTION_CALL_H
#define AST_EXPRESSION_FUNCTION_CALL_H

#include "ast/expression/base.h"
#include "ast/expression.h"
#include "parser.h"

namespace AST::ExpressionImpl {

class FunctionCall : public Base {
    const NonTerminals::Expression::FunctionCall &parserFunctionCall;
    std::optional<Expression> functionId;
    std::vector<Expression> arguments;
    std::string functionName;

public:
    explicit FunctionCall( const NonTerminals::Expression::FunctionCall &parserFunctionCall );

protected:
    void buildASTImpl( LookupContext &lookupContext, ExpectedResult expectedResult ) override;
    ExpressionId codeGenImpl( PracticalSemanticAnalyzer::FunctionGen *functionGen ) override;

private:
    void resolveOverloads(
            LookupContext &lookupContext,
            ExpectedResult expectedResult,
            const std::unordered_map<const Tokenizer::Token *, LookupContext::Function::Definition> &overloads
            );

    void buildActualCall(
            LookupContext &lookupContext, ExpectedResult expectedResult,
            const LookupContext::Function::Definition *definition );
};

} // namespace AST::ExpressionImpl

#endif // AST_EXPRESSION_FUNCTION_CALL_H
