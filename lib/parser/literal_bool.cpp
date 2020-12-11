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

size_t LiteralBool::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

    const Tokenizer::Token *currentToken = nextToken(source, tokensConsumed, "EOF while parsing literal");

    switch( currentToken->token ) {
    case Tokenizer::Tokens::RESERVED_FALSE:
        value = false;
        break;
    case Tokenizer::Tokens::RESERVED_TRUE:
        value = true;
        break;
    default:
        ABORT()<<"Unreachable code reached";
    }

    token = currentToken;

    RULE_LEAVE();
}

} // namespace NonTerminals
