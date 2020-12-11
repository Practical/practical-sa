/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * This file is file is copyright (C) 2020 by its authors.
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */

#include "parser/literal_string.h"

#include "parser_internal.h"

#include <practical/errors.h>

namespace NonTerminals {

using namespace InternalNonTerminals;

size_t LiteralString::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

    token = &expectToken(
            Tokenizer::Tokens::LITERAL_STRING, source, tokensConsumed,
            "Expected null literal", "EOF while parsing literal");

    static const std::unordered_map<
            State,
            State (LiteralString::*)( String, int &, const SourceLocation & )
        > stateParsers
    {
        { State::None, &LiteralString::parserNone },
        { State::Backslash, &LiteralString::parserBackslash }
    };

    ASSERT( token->token == Tokenizer::Tokens::LITERAL_STRING );
    ASSERT( value.empty() );

    String body = token->text;
    ASSERT( body.size()>2 );
    ASSERT( body[0]=='"' );
    ASSERT( body[body.size()-1]=='"' );

    SourceLocation location = token->location;

    // Strip leading and trailing quotes
    body = body.subslice( 1, body.size()-1 );
    ++location;

    ASSERT( state == State::None );

    int stateData = 0;
    while( body.size()>0 ) {
        state = (this ->* stateParsers.at(state))( body, stateData, location );
        body = body.subslice(1);
        ++location;
    }

    RULE_LEAVE();
}

LiteralString::State LiteralString::parserNone(
        String source, int &stateData, const SourceLocation &location )
{
    switch( source[0] ) {
    case '\\':
        return State::Backslash;
    }

    value += source[0];
    return State::None;
}

LiteralString::State LiteralString::parserBackslash(
        String source, int &stateData, const SourceLocation &location )
{
    switch( source[0] ) {
    case '\'':
    case '"':
    case '?':
    case '\\':
        value += source[0];
        return State::None;
    case '0':
        value += char(0); // NUL (Null)
        return State::None;
    case 'a':
        value += 7; // BEL (Bell)
        return State::None;
    case 'b':
        value += 8; // BS (Backspace)
        return State::None;
    case 't':
        value += 9; // HT (Horizontal tab)
        return State::None;
    case 'n':
        value += 10; // NL (Newline)
        return State::None;
    case 'v':
        value += 11; // VT (Vertical tab)
        return State::None;
    case 'f':
        value += 12; // FF (Form feed)
        return State::None;
    case 'r':
        value += 13; // CR (Carriage return)
        return State::None;
    case 'x':
        stateData = 0;
        value += char(0);
        return State::Hex;
    }

    throw InvalidEscapeSequence(location);
}

} // namespace NonTerminals
