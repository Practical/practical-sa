/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef PARSER_MODULE_H
#define PARSER_MODULE_H

#include "parser/base.h"
#include "parser/struct.h"
#include "parser.h"

namespace NonTerminals {
    struct Module : public NonTerminal {
        std::vector< FuncDef > functionDefinitions;
        std::vector< FuncDecl > functionDeclarations;
        std::vector< StructDef > structureDefinitions;
        std::vector< Tokenizer::Token > tokens;

        void parse(String source);
        size_t parse(Slice<const Tokenizer::Token> source) override final;
        String getName() const {
            return toSlice("__main");
        }
    };
} // NonTerminals

#endif // PARSER_MODULE_H
