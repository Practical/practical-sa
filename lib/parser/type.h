/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef PARSER_TYPE_H
#define PARSER_TYPE_H

#include "parser/identifier.h"

namespace NonTerminals {

struct Type : public NonTerminal {
    struct Pointer {
        std::unique_ptr< const Type > pointed;
        const Tokenizer::Token *token = nullptr;

        Pointer( std::unique_ptr< const Type > pointed, const Tokenizer::Token *token ) :
            pointed(std::move(pointed)), token(token)
        {}
    };
    std::variant<std::monostate, Identifier, Pointer> type;

    size_t parse(Slice<const Tokenizer::Token> source) override final;
    SourceLocation getLocation() const;
};

} // namespace NonTerminals

#endif // PARSER_TYPE_H
