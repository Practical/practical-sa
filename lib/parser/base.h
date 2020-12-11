/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef PARSER_BASE_H
#define PARSER_BASE_H

#include "tokenizer.h"

namespace NonTerminals {

struct NonTerminal {
protected:
    Slice<const Tokenizer::Token> parsedSlice;

public:
    // This function is not really virtual. It's used this way to force all children to have the same signature
    // Returns how many tokens were consumed
    // Throws parser_error if fails to parse
    virtual size_t parse(Slice<const Tokenizer::Token> source) = 0;

    virtual ~NonTerminal() {}

    Slice<const Tokenizer::Token> getNTTokens() const {
        return parsedSlice;
    }
};

} // namespace NonTerminals

#endif // PARSER_BASE_H
