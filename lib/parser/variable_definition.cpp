/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "parser/variable_definition.h"

#include "parser_internal.h"

#include <practical/errors.h>

namespace NonTerminals {

using namespace InternalNonTerminals;

size_t VariableDeclBody::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

    tokensConsumed += name.parse(source);
    expectToken( Tokenizer::Tokens::OP_COLON, source, tokensConsumed, "Expected \":\" after variable name", "Unexpected EOF" );
    tokensConsumed += type.parse( source.subslice(tokensConsumed) );

    RULE_LEAVE();
}

size_t VariableDefinition::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

    expectToken( Tokenizer::Tokens::RESERVED_DEF, source, tokensConsumed, "Variable definition does not start with def keyword",
            "Unexpected EOF" );

    tokensConsumed += body.parse(source.subslice(tokensConsumed));

    try {
        size_t provisionalConsumed = tokensConsumed;

        expectToken( Tokenizer::Tokens::OP_ASSIGN, source, provisionalConsumed, "", "" );

        // XXX parse outside the catch
        Expression initValue;
        provisionalConsumed += initValue.parse( source.subslice(provisionalConsumed) );

        this->initValue = safenew<Expression>( std::move(initValue) );
        tokensConsumed = provisionalConsumed;
    } catch( parser_error &err ) {
        EXCEPTION_CAUGHT(err);
    }

    RULE_LEAVE();
}

} // namespace NonTerminals
