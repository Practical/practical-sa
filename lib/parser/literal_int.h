/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * This file is file is copyright (C) 2020 by its authors.
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef PARSER_LITERAL_INT_H
#define PARSER_LITERAL_INT_H

#include "parser/base.h"

namespace NonTerminals {

struct LiteralInt : public NonTerminal {
    const Tokenizer::Token *token = nullptr;
    LongEnoughInt value = 0;

    size_t parse(Slice<const Tokenizer::Token> source) override final;

private:
    void parseBinary();
    void parseOctal();
    void parseDecimal();
    void parseHexadecimal();
};

} // namespace NonTerminals

#endif // PARSER_LITERAL_INT_H
