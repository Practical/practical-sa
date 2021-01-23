/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef PARSER_VARIABLE_DEFINITION_H
#define PARSER_VARIABLE_DEFINITION_H

#include "parser/base.h"
#include "parser/type.h"

#include <memory>

namespace NonTerminals {

struct Expression;

struct VariableDeclBody : public NonTerminal {
    Identifier name;
    Type type;

    size_t parse(Slice<const Tokenizer::Token> source) override final;
};

struct VariableDefinition : public NonTerminal {
    VariableDeclBody body;
    std::unique_ptr<Expression> initValue;

    size_t parse(Slice<const Tokenizer::Token> source) override final;
};

} // NonTerminals

#endif // PARSER_VARIABLE_DEFINITION_H
