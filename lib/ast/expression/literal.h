/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_EXPRESSION_LITERAL_H
#define AST_EXPRESSION_LITERAL_H

#include "ast/expression/base.h"
#include "parser.h"

namespace AST::ExpressionImpl {

class Literal final : public Base {
    // Members
    const NonTerminals::Literal &parserLiteral;

public:
    explicit Literal( const NonTerminals::Literal &parserLiteral );

    SourceLocation getLocation() const override;

protected:
    void buildASTImpl(
            LookupContext &lookupContext, ExpectedResult expectedResult, Weight &weight, Weight weightLimit
        ) override;
    ExpressionId codeGenImpl( PracticalSemanticAnalyzer::FunctionGen *functionGen ) const override;

private:
    void buildAstInt(
            const NonTerminals::LiteralInt &literal, Weight &weight, Weight weightLimit,
            ExpectedResult expectedResult );
    void buildAstBool(
            const NonTerminals::LiteralBool &literal, Weight &weight, Weight weightLimit,
            ExpectedResult expectedResult );
    void buildAstPointer(
            const NonTerminals::LiteralPointer &literal, Weight &weight, Weight weightLimit,
            ExpectedResult expectedResult );
    void buildAstString(
            const NonTerminals::LiteralString &literal, Weight &weight, Weight weightLimit,
            ExpectedResult expectedResult );

    ExpressionId codeGenInt(
            const NonTerminals::LiteralInt &literal,
            PracticalSemanticAnalyzer::FunctionGen *functionGen ) const;
    ExpressionId codeGenBool(
            const NonTerminals::LiteralBool &literal,
            PracticalSemanticAnalyzer::FunctionGen *functionGen ) const;
    ExpressionId codeGenString(
            const NonTerminals::LiteralString &literal,
            PracticalSemanticAnalyzer::FunctionGen *functionGen ) const;
    ExpressionId codeGenPointer(
            const NonTerminals::LiteralPointer &literal,
            PracticalSemanticAnalyzer::FunctionGen *functionGen ) const;
};

} // namespace AST::ExpressionImpl

#endif // AST_EXPRESSION_LITERAL_H
