/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "parser/identifier.h"

#include "parser_internal.h"

namespace NonTerminals {

using namespace InternalNonTerminals;

size_t Identifier::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

    identifier = &expectToken(Tokenizer::Tokens::IDENTIFIER, source, tokensConsumed, "Expected an identifier",
            "EOF while parsing an identifier" );

    RULE_LEAVE();
}

} // namespace NonTerminals
