/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "parser/module.h"

#include "parser_internal.h"

#include <practical/errors.h>

namespace NonTerminals {

using namespace InternalNonTerminals;

void Module::parse(String source) {
    tokens = Tokenizer::Tokenizer::tokenize(source);
    parse(tokens);
}

size_t Module::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

    skipWS(source, tokensConsumed);
    while( tokensConsumed<source.size() ) {
        const Tokenizer::Token *currentToken = wishForToken(
                Tokenizer::Tokens::RESERVED_DEF,
                source, tokensConsumed,
                false);
        if( currentToken != nullptr ) {
            FuncDef func;

            tokensConsumed += func.parse( source.subslice(tokensConsumed) );
            functionDefinitions.emplace_back( std::move(func) );

            skipWS(source, tokensConsumed);
            continue;
        }

        currentToken = wishForToken(
                Tokenizer::Tokens::RESERVED_DECL,
                source, tokensConsumed,
                false);
        if( currentToken != nullptr ) {
            FuncDecl func;

            tokensConsumed += func.parse( source.subslice(tokensConsumed) );
            functionDeclarations.emplace_back( std::move(func) );

            skipWS(source, tokensConsumed);
            continue;
        }

        currentToken = wishForToken(
                Tokenizer::Tokens::RESERVED_STRUCT,
                source, tokensConsumed,
                false);
        if( currentToken != nullptr ) {
            StructDef strct;

            tokensConsumed += strct.parse( source.subslice(tokensConsumed) );
            structureDefinitions.emplace_back( std::move(strct) );

            skipWS(source, tokensConsumed);
            continue;
        }

        throw parser_error("Unidentified statement in global context", source[tokensConsumed].location );
    }

    RULE_LEAVE();
}

} // namespace NonTerminals
