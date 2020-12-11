/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * This file is file is copyright (C) 2020 by its authors.
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef PARSER_LITERAL_STRING_H
#define PARSER_LITERAL_STRING_H

#include "parser/base.h"

namespace NonTerminals {

struct LiteralString : public NonTerminal {
public:
    size_t parse(Slice<const Tokenizer::Token> source) override final;

private:
    enum class State {
        None,
        Backslash,
        Hex,
    };

    State parserNone( String source, int &stateData, const SourceLocation &location );
    State parserBackslash( String source, int &stateData, const SourceLocation &location );

    // Members
    State state = State::None;
public:
    const Tokenizer::Token *token = nullptr;
    std::string value;
};

} // namespace NonTerminals

#endif // PARSER_LITERAL_STRING_H
