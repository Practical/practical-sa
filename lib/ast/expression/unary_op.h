/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_EXPRESSION_UNARY_OP_H
#define AST_EXPRESSION_UNARY_OP_H

#include "ast/expression/address_of.h"
#include "ast/expression/overload_resolver.h"
#include "ast/expression.h"
#include "parser.h"

namespace AST::ExpressionImpl {

class UnaryOp final : public Base {
    const NonTerminals::Expression::UnaryOperator &parserOp;
    std::variant<std::monostate, OverloadResolver, AddressOf> body;

public:
    static void init(LookupContext &builtinCtx);

    explicit UnaryOp( const NonTerminals::Expression::UnaryOperator &parserOp );

    SourceLocation getLocation() const override;

protected:
    void buildASTImpl(
            LookupContext &lookupContext, ExpectedResult expectedResult, Weight &weight, Weight weightLimit
        ) override;
    ExpressionId codeGenImpl( PracticalSemanticAnalyzer::FunctionGen *functionGen ) const override;

private:
    void buildASTFromTemplate(
            OverloadResolver &resolver, LookupContext &lookupContext, ExpectedResult expectedResult,
            Weight &weight, Weight weightLimit
        );
};

} // namespace AST::ExpressionImpl

#endif // AST_EXPRESSION_UNARY_OP_H
