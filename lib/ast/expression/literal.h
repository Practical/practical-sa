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

        virtual ExpressionId codeGen(
                const Literal *owner, PracticalSemanticAnalyzer::FunctionGen *functionGen ) const = 0;
    };

    // Members
    const NonTerminals::Literal &parserLiteral;
    std::unique_ptr<Impl> impl;

public:
    explicit Literal( const NonTerminals::Literal &parserLiteral );

    SourceLocation getLocation() const override;

protected:
    void buildASTImpl(
            LookupContext &lookupContext, ExpectedResult expectedResult, Weight &weight, Weight weightLimit
        ) override;
    ExpressionId codeGenImpl( PracticalSemanticAnalyzer::FunctionGen *functionGen ) const override;

private:
    class LiteralInt final : public Impl {
        LongEnoughInt result = 0;

    public:
        ExpressionId codeGen( const Literal *owner, PracticalSemanticAnalyzer::FunctionGen *functionGen ) const override;

        void parseInt10( Literal *owner, Weight &weight, Weight weightLimit, ExpectedResult expectedResult );
        void parseInt( Literal *owner, Weight &weight, Weight weightLimit, ExpectedResult expectedResult );
    };

    class LiteralBool final : public Impl {
        bool result = false;

    public:
        ExpressionId codeGen( const Literal *owner, PracticalSemanticAnalyzer::FunctionGen *functionGen ) const override;

        void parseBool( Literal *owner, Weight &weight, Weight weightLimit, ExpectedResult expectedResult );
    };

    class LiteralString final : public Impl {
        enum class State {
            None,
            Backslash,
            Hex,
        };

    public:
        ExpressionId codeGen( const Literal *owner, PracticalSemanticAnalyzer::FunctionGen *functionGen ) const override;

        void parse( Literal *owner, Weight &weight, Weight weightLimit, ExpectedResult expectedResult );

    private:
        State parserNone( String source, int &stateData, const SourceLocation &location );
        State parserBackslash( String source, int &stateData, const SourceLocation &location );

        // Members
        std::string result;
        State state = State::None;
    };

    class LiteralNull final : public Impl {
    public:
        ExpressionId codeGen( const Literal *owner, PracticalSemanticAnalyzer::FunctionGen *functionGen ) const override;
        void buildAst( Literal *owner, Weight &weight, Weight weightLimit, ExpectedResult expectedResult );
    };
};

} // namespace AST::ExpressionImpl

#endif // AST_EXPRESSION_LITERAL_H
