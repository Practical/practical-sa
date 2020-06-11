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
    class Impl {
    public:
        virtual ~Impl() {}

        virtual ExpressionId codeGen( Literal *owner, PracticalSemanticAnalyzer::FunctionGen *functionGen ) = 0;
    };

    // Members
    const NonTerminals::Literal &parserLiteral;
    std::unique_ptr<Impl> impl;

public:
    explicit Literal( const NonTerminals::Literal &parserLiteral );

protected:
    void buildASTImpl(
            LookupContext &lookupContext, ExpectedResult expectedResult, unsigned &weight, unsigned weightLimit
        ) override;
    ExpressionId codeGenImpl( PracticalSemanticAnalyzer::FunctionGen *functionGen ) override;

private:
    class LiteralInt final : public Impl {
        LongEnoughInt result = 0;

    public:
        ExpressionId codeGen( Literal *owner, PracticalSemanticAnalyzer::FunctionGen *functionGen ) override;

        void parseInt10( Literal *owner, unsigned &weight, unsigned weightLimit, ExpectedResult expectedResult );
        void parseInt( Literal *owner, unsigned &weight, unsigned weightLimit, ExpectedResult expectedResult );
    };
};

} // namespace AST::ExpressionImpl

#endif // AST_EXPRESSION_LITERAL_H
