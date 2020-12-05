/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_EXPRESSION_OVERLOAD_RESOLVER_H
#define AST_EXPRESSION_OVERLOAD_RESOLVER_H

#include "ast/expression/base.h"
#include "ast/expected_result.h"
#include "ast/lookup_context.h"

namespace AST::ExpressionImpl {

class OverloadResolver {
    std::vector<Expression> arguments;
    const LookupContext::Function::Definition *definition;

public:
    void resolveOverloads(
            LookupContext &lookupContext,
            ExpectedResult expectedResult,
            const LookupContext::Function::OverloadsContainer &overloads,
            Weight &weight,
            Weight weightLimit,
            ExpressionMetadata &metadata,
            Slice<const NonTerminals::Expression *const> parserArguments,
            const Tokenizer::Token *sourceLocation
        );

    const FunctionTypeImpl &getType() const;

    ExpressionId codeGen( PracticalSemanticAnalyzer::FunctionGen *functionGen ) const;

private:
    void buildActualCall(
            LookupContext &lookupContext, Weight &weight, Weight weightLimit,
            const LookupContext::Function::Definition *definition,
            ExpressionMetadata &metadata,
            Slice<const NonTerminals::Expression *const> parserArguments );

    void resolveOverloadsByReturn(
            LookupContext &lookupContext,
            ExpectedResult expectedResult,
            const LookupContext::Function::OverloadsContainer &overloads,
            Weight &weight,
            Weight weightLimit,
            ExpressionMetadata &metadata,
            Slice<const NonTerminals::Expression *const> parserArguments,
            const Tokenizer::Token *sourceLocation
        );
    void resolveOverloadsByArguments(
            LookupContext &lookupContext,
            const LookupContext::Function::OverloadsContainer &overloads,
            Weight &weight,
            Weight weightLimit,
            ExpressionMetadata &metadata,
            Slice<const NonTerminals::Expression *const> parserArguments,
            const Tokenizer::Token *sourceLocation
        );
    void findBestOverloadByArgument(
            LookupContext &lookupContext,
            Slice< const LookupContext::Function::Definition * > overloads,
            Weight &weight,
            Weight weightLimit,
            ExpressionMetadata &metadata,
            Slice<const NonTerminals::Expression *const> parserArguments,
            const Tokenizer::Token *sourceLocation
        );
};

} // namespace AST::ExpressionImpl

#endif // AST_EXPRESSION_OVERLOAD_RESOLVER_H
