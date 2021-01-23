/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "parser/struct.h"

#include "parser_internal.h"

namespace NonTerminals {

using namespace InternalNonTerminals;

size_t StructDef::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

    keyword = &expectToken( Tokenizer::Tokens::RESERVED_STRUCT, source, tokensConsumed,
            "Struct definition must start with the keyword `struct`", "EOF looking for struct definition" );
    tokensConsumed += identifier.parse( source.subslice(tokensConsumed) );

    expectToken( Tokenizer::Tokens::BRACKET_CURLY_OPEN, source, tokensConsumed,
            "Struct definition starts with `{`", "EOF looking for `{` in struct definition" );

    const Tokenizer::Token *closingBracket =
            wishForToken( Tokenizer::Tokens::BRACKET_CURLY_CLOSE, source, tokensConsumed, true );

    while(closingBracket==nullptr) {
        VariableDefinition def;
        tokensConsumed += def.parse( source.subslice(tokensConsumed) );
        expectToken( Tokenizer::Tokens::SEMICOLON, source, tokensConsumed,
                "Struct definitions must end with semicolon", "EOF while defining a struct" );

        definitions.emplace_back( std::move(def) );

        closingBracket = wishForToken( Tokenizer::Tokens::BRACKET_CURLY_CLOSE, source, tokensConsumed, true );
    }

    RULE_LEAVE();
}

} // namespace NonTerminals
