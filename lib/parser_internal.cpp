#include "parser_internal.h"

#include <practical/errors.h>

#if VERBOSE_PARSING
size_t PARSER_RECURSION_DEPTH;
#endif

namespace InternalNonTerminals {

bool skipWS(Slice<const Tokenizer::Token> source, size_t &index) {
    bool moved = false;

    while( index<source.size() &&
            (source[index].token == Tokenizer::Tokens::WS || source[index].token == Tokenizer::Tokens::COMMENT_LINE_END ||
             source[index].token == Tokenizer::Tokens::COMMENT_MULTILINE) )
    {
        index++;
        moved = true;
    }

    return moved;
}

// Consumes the next token, optionally reporting EOF
const Tokenizer::Token *nextToken(Slice<const Tokenizer::Token> source, size_t &index, const char *msg) {
    skipWS(source, index);

    if( index==source.size() ) {
        if( msg==nullptr )
            return nullptr;
        else
            throw parser_error(msg, SourceLocation());
    }

    return &source[index++];
}

const Tokenizer::Token &expectToken(
        Tokenizer::Tokens expected, Slice<const Tokenizer::Token> source, size_t &index, const char *mismatchMsg,
        const char *eofMsg)
{
    if( eofMsg==nullptr )
        eofMsg=mismatchMsg;

    const Tokenizer::Token *currentToken = nextToken( source, index, eofMsg );
    ASSERT( currentToken!=nullptr ) << "Unexpected EOF condition when searching for token";

    if( currentToken->token!=expected ) {
        index--;
        throw parser_error(mismatchMsg, currentToken->location);
    }

    return *currentToken;
}

bool wishForToken(
        Tokenizer::Tokens expected,
        Slice<const Tokenizer::Token> source,
        size_t &index,
        bool consumeTokens)
{
    size_t indexCopy = index;
    skipWS( source, indexCopy );

    if( indexCopy>=source.size() ) {
        return false;
    }

    if( source[indexCopy].token == expected ) {
        if( consumeTokens )
            index = indexCopy + 1;

        return true;
    }

    return false;
}

size_t ExpressionOrStatement::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

    if( wishForToken( Tokenizer::Tokens::BRACKET_CURLY_OPEN, source, tokensConsumed, false ) ) {
        CompoundExpressionOrStatement parsed;
        tokensConsumed = parsed.parse(source);

        if( parsed.isStatement() )
            content.emplace<Statement>( safenew<CompoundStatement>(parsed.removeStatement()) );
        else
            content.emplace<NonTerminals::Expression>( safenew<CompoundExpression>(parsed.removeExpression()) );

        RULE_LEAVE();
    }

    Expression expression;
    tokensConsumed = expression.parse(source);

    if( wishForToken( Tokenizer::Tokens::SEMICOLON, source, tokensConsumed ) )
        content.emplace<Statement>( std::move(expression) );
    else
        content.emplace<Expression>( std::move(expression) );

    RULE_LEAVE();
}

size_t ConditionalExpressionOrStatement::parse(Slice<const Tokenizer::Token> source ) {
    return parse( source, ExpectedResult::Unknown );
}

size_t ConditionalExpressionOrStatement::parse(Slice<const Tokenizer::Token> source, ExpectedResult result ) {
    RULE_ENTER(source);

    auto ifToken = expectToken(
            Tokenizer::Tokens::RESERVED_IF, source, tokensConsumed,
            "Condition must start with 'if'", "EOF searching for condition" );

    Expression condition;

    expectToken( Tokenizer::Tokens::BRACKET_ROUND_OPEN, source, tokensConsumed, "Expecting '(' after if" );
    tokensConsumed += condition.parse( source.subslice(tokensConsumed) );
    expectToken( Tokenizer::Tokens::BRACKET_ROUND_CLOSE, source, tokensConsumed, "Expecting ')' at end of condition" );

    ExpressionOrStatement ifClause;
    tokensConsumed += ifClause.parse( source.subslice(tokensConsumed) );

    std::unique_ptr<ExpressionOrStatement> elseClause;
    if( wishForToken(Tokenizer::Tokens::RESERVED_ELSE, source, tokensConsumed) ) {
        elseClause = safenew<ExpressionOrStatement>();
        tokensConsumed += elseClause->parse( source.subslice(tokensConsumed) );
    }

    if( result==ExpectedResult::Unknown )
        result = ifClause.isStatement() ? ExpectedResult::Statement : ExpectedResult::Expression;

    // Make sure parsing matches what we expect
    switch(result) {
    case ExpectedResult::Statement:
        {
            if( ! ifClause.isStatement() )
                throw parser_error(
                        "condition must have statement (not expression) as \"then\" clause", ifToken.location);

            if( elseClause && !elseClause->isStatement() )
                throw parser_error(
                        "condition must have statement (not expression) as \"else\" clause", ifToken.location);

            auto &statement=this->condition.emplace<Statement::ConditionalStatement>();
            statement.condition=std::move(condition);
            statement.ifClause = safenew<Statement>( ifClause.removeStatement() );
            if( elseClause )
                statement.elseClause = safenew<Statement>( elseClause->removeStatement() );
        }
        break;
    case ExpectedResult::Expression:
        {
            if( ifClause.isStatement() )
                throw parser_error(
                        "condition must have expression (not statement) as \"then\" clause", ifToken.location);

            if( !elseClause )
                throw parser_error(
                        "conditional expression must have an \"else\" clause", ifToken.location);

            if( elseClause->isStatement() )
                throw parser_error(
                        "condition must have expression (not statement) as \"else\" clause", ifToken.location);

            auto &expression = this->condition.emplace<ConditionalExpression>();
            expression.condition = std::move(condition);
            expression.ifClause = ifClause.removeExpression();
            expression.elseClause = elseClause->removeExpression();

            if(
                    ! std::get_if< std::unique_ptr<CompoundExpression> >(& expression.ifClause.value) ||
                    ! std::get_if< std::unique_ptr<CompoundExpression> >(& expression.elseClause.value)
              )
            {
                throw parser_error(
                        "Conditional expression must use compound expressions for \"then\" and \"else\" clauses",
                        ifToken.location);
            }
        }
        break;
    case ExpectedResult::Unknown:
        ABORT()<<"Unreachable code reached";
        break;
    }

    RULE_LEAVE();
}

bool CompoundExpressionOrStatement::isStatement() const {
    ASSERT( content.index()!=0 )<<
            "Tried to get statement/expression from CompoundExpressionOrStatement containing neither";
    return content.index()==2;
}

CompoundStatement CompoundExpressionOrStatement::removeStatement() {
    return CompoundStatement{ std::move( std::get<CompoundStatement>(content) ) };
}

CompoundExpression CompoundExpressionOrStatement::removeExpression() {
    return CompoundExpression( std::move( std::get<CompoundExpression>(content) ) );
}

size_t CompoundExpressionOrStatement::parseInternal(Slice<const Tokenizer::Token> source, ParseType parseType) {
    RULE_ENTER(source);

    expectToken( Tokenizer::Tokens::BRACKET_CURLY_OPEN, source, tokensConsumed, "Expected {",
            "EOF while parsing compound statement" );

    StatementList statementList;
    tokensConsumed += statementList.parse(source.subslice(tokensConsumed));

    try {
        if( parseType!=ParseType::Statement ) {
            Expression expression;
            tokensConsumed += expression.parse(source.subslice(tokensConsumed));

            parseType=ParseType::Expression;

            content.emplace<CompoundExpression>( std::move(statementList), std::move(expression) );
        }
    } catch( parser_error &err ) {
        if( parseType==ParseType::Expression )
            throw;

        EXCEPTION_CAUGHT(err);

        parseType = ParseType::Statement;
    }

    ASSERT( parseType!=ParseType::Either );
    if( parseType==ParseType::Statement ) {
        content.emplace<CompoundStatement>( std::move(statementList) );
    }

    expectToken( Tokenizer::Tokens::BRACKET_CURLY_CLOSE, source, tokensConsumed, "Expected }",
            "Unmatched {" );

    RULE_LEAVE();
}

} // End namespace InternalNonTerminals
