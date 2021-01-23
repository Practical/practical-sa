/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef PARSER_STRUCT_H
#define PARSER_STRUCT_H

#include "parser/base.h"
#include "parser/variable_definition.h"

namespace NonTerminals {

struct StructDef : public NonTerminal {
    const Tokenizer::Token *keyword = nullptr;
    Identifier identifier;
    std::vector<VariableDefinition> definitions;

    size_t parse(Slice<const Tokenizer::Token> source) override final;
    SourceLocation getLocation() const {
        ASSERT(keyword != nullptr) << "Dereferencing an unparsed struct";
        return keyword->location;
    }
};

} // NonTerminals

#endif // PARSER_STRUCT_H
