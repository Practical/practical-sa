/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef PARSER_INTERNAL_H
#define PARSER_INTERNAL_H

#include "parser.h"

#if VERBOSE_PARSING
extern size_t PARSER_RECURSION_DEPTH;

#define RULE_ENTER(source) \
    size_t RECURSION_CURRENT_DEPTH = PARSER_RECURSION_DEPTH++; \
    for( size_t I=0; I<RECURSION_CURRENT_DEPTH; ++I ) std::cout<<"  "; \
    if( source.size()>0 ) \
        std::cout<<"Processing " << __PRETTY_FUNCTION__ << " of " << source[0].token << " at " << source[0].location << "\n" ;\
    else\
        std::cout<<"Processing " << __PRETTY_FUNCTION__ << " at EOF\n" ;\
    size_t tokensConsumed = 0

#define RULE_LEAVE() \
    PARSER_RECURSION_DEPTH = RECURSION_CURRENT_DEPTH; \
    for( size_t I=0; I<RECURSION_CURRENT_DEPTH; ++I ) std::cout<<"  "; \
    std::cout<<"Leaving " << __PRETTY_FUNCTION__ << " consumed " << tokensConsumed << "\n"; \
    this->parsedSlice = source.subslice(0, tokensConsumed); \
    return tokensConsumed

#define EXCEPTION_CAUGHT(ex) \
    PARSER_RECURSION_DEPTH = RECURSION_CURRENT_DEPTH + 1; \
    for( size_t I=0; I<RECURSION_CURRENT_DEPTH; ++I ) std::cout<<"  "; \
    std::cout<< __PRETTY_FUNCTION__ << " caught " << ex.what() << "\n"

#else

#define RULE_ENTER(source) size_t tokensConsumed = 0
#define RULE_LEAVE() \
    this->parsedSlice = source.subslice(0, tokensConsumed); \
    return tokensConsumed
#define EXCEPTION_CAUGHT(ex)

#endif

namespace InternalNonTerminals {
    using namespace NonTerminals;

    enum class ExpectedResult {
        Unknown,
        Statement,
        Expression,
    };

    bool skipWS(Slice<const Tokenizer::Token> source, size_t &index);
    const Tokenizer::Token *nextToken(Slice<const Tokenizer::Token> source, size_t &index, const char *msg = nullptr);
    const Tokenizer::Token &expectToken(
            Tokenizer::Tokens expected, Slice<const Tokenizer::Token> source, size_t &index, const char *mismatchMsg,
            const char *eofMsg = nullptr);

    const Tokenizer::Token *wishForToken(
            Tokenizer::Tokens expected,
            Slice<const Tokenizer::Token> source,
            size_t &index,
            bool consumeTokens = true);

    struct ExpressionOrStatement : public NonTerminal {
        std::variant<std::monostate, Expression, Statement> content;

        size_t parse(Slice<const Tokenizer::Token> source) override final;

        bool isStatement() const {
            ASSERT( content.index()!=0 )<<
                    "Tried to get statement/expression from CompoundExpressionOrStatement containing neither";
            return content.index()==2;
        }

        bool isValid() const {
            return content.index()!=0;
        }

        Expression removeExpression() {
            return Expression( std::move( std::get<Expression>(content) ) );
        }
        Statement removeStatement() {
            return Statement( std::move( std::get<Statement>(content) ) );
        }
    };

    struct ConditionalExpressionOrStatement : public NonTerminal {
        std::variant<
                std::monostate,
                ConditionalExpression,
                Statement::ConditionalStatement
            > condition;

        size_t parse(Slice<const Tokenizer::Token> source) override final;
        size_t parse(Slice<const Tokenizer::Token> source, ExpectedResult result);

        bool isStatement() const {
            ASSERT( condition.index()!=0 )<<
                    "Tried to get statement/expression from ConditionalExpressionOrStatement containing neither";
            return condition.index()==2;
        }

        ConditionalExpression removeExpression() {
            return ConditionalExpression( std::move( std::get<ConditionalExpression>(condition) ) );
        }
        Statement::ConditionalStatement removeStatement() {
            return Statement::ConditionalStatement( std::move( std::get<Statement::ConditionalStatement>(condition) ) );
        }
    };

    struct CompoundExpressionOrStatement : public NonTerminal {
        std::variant<std::monostate, CompoundExpression, CompoundStatement> content;

        size_t parse(Slice<const Tokenizer::Token> source) override final {
            return parseInternal(source, ParseType::Either);
        }
        size_t parseExpression(Slice<const Tokenizer::Token> source) {
            return parseInternal(source, ParseType::Expression);
        }
        size_t parseStatement(Slice<const Tokenizer::Token> source) {
            return parseInternal(source, ParseType::Statement);
        }

        bool isStatement() const;
        CompoundStatement removeStatement();
        CompoundExpression removeExpression();

    private:
        enum class ParseType { Either, Statement, Expression };
        size_t parseInternal(Slice<const Tokenizer::Token> source, ParseType parseType);
    };
} // InternalNonTerminals

#endif // PARSER_INTERNAL_H
