/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * This file is file is copyright (C) 2020 by its authors.
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "parser/literal_int.h"

#include "parser_internal.h"

#include <practical/errors.h>

namespace NonTerminals {

using namespace InternalNonTerminals;

size_t LiteralInt::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

    const Tokenizer::Token *currentToken = nextToken(source, tokensConsumed, "EOF while parsing literal");

    switch( currentToken->token ) {
    case Tokenizer::Tokens::LITERAL_INT_2:
        token = currentToken;
        parseBinary();
        break;
    case Tokenizer::Tokens::LITERAL_INT_8:
        token = currentToken;
        parseOctal();
        break;
    case Tokenizer::Tokens::LITERAL_INT_10:
        token = currentToken;
        parseDecimal();
        break;
    case Tokenizer::Tokens::LITERAL_INT_16:
        token = currentToken;
        parseHexadecimal();
        break;
    default:
        throw parser_error("Invalid integer literal", currentToken->location);
    }

    RULE_LEAVE();
}

void LiteralInt::parseBinary() {
    ABORT()<<"TODO implement binary literals";
}

void LiteralInt::parseOctal() {
    ABORT()<<"TODO implement octal literals";
}

void LiteralInt::parseDecimal() {
    value = 0;

    for( char c: token->text ) {
        static constexpr LongEnoughInt
                LimitDivided = std::numeric_limits<LongEnoughInt>::max() / 10,
                LimitTruncated = LimitDivided * 10,
                LimitLastDigit = std::numeric_limits<LongEnoughInt>::max() % 10;

        if( c=='_' )
            continue;

        if( value > LimitDivided )
            throw IllegalLiteral( "Literal integer too big", token->location );

        ASSERT( c>='0' && c<='9' ) << "Decimal literal has character '"<<c<<"' out of allowed range";
        value *= 10;
        if( value == LimitTruncated && c-'0'>LimitLastDigit )
            throw IllegalLiteral( "Literal integer too big", token->location );

        value += c-'0';
    }

}

void LiteralInt::parseHexadecimal() {
    ABORT()<<"TODO implement hexadecimal literals";
}

} // namespace NonTerminals
