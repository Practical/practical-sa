/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef PARSER_IDENTIFIER_H
#define PARSER_IDENTIFIER_H

#include "parser/base.h"

#include "asserts.h"

namespace NonTerminals {

struct Identifier : public NonTerminal {
    const Tokenizer::Token *identifier = nullptr;

    size_t parse(Slice<const Tokenizer::Token> source) override final;

    String getName() const {
        return identifier->text;
    }

    SourceLocation getLocation() const {
        ASSERT(identifier != nullptr) << "Dereferencing an unparsed identifier";
        return identifier->location;
    }
};

} // NonTerminals

#endif // PARSER_IDENTIFIER_H
