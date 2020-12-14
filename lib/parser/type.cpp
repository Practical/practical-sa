/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "parser/type.h"

#include "parser_internal.h"

namespace NonTerminals {

using namespace InternalNonTerminals;

Type::Array::Array( std::unique_ptr< const Type > elementType, const Tokenizer::Token *token ) :
    elementType( std::move(elementType) ),
    token(token)
{}

size_t Type::parse(Slice<const Tokenizer::Token> source) {
    RULE_ENTER(source);

    Identifier &id = type.emplace<Identifier>();

    tokensConsumed = id.parse( source );

    bool done=false;

    do {
        size_t provisionalyConsumed = 0;
        const Tokenizer::Token *token = nextToken( source.subslice(tokensConsumed), provisionalyConsumed, nullptr );
        if( !token )
            break;

        switch( token->token ) {
        case Tokenizer::Tokens::BRACKET_SQUARE_OPEN:
            {
                auto elementType = std::make_unique<Type>();
                elementType->type = std::move(type);

                Array &array = type.emplace< Array >( std::move(elementType), token );
                provisionalyConsumed += array.dimension.parse( source.subslice(tokensConsumed + provisionalyConsumed) );
                expectToken(
                        Tokenizer::Tokens::BRACKET_SQUARE_CLOSE, source.subslice(tokensConsumed), provisionalyConsumed,
                        "Array type with no closing bracket", "Array type with no closing bracket" );
            }
            break;
        case Tokenizer::Tokens::OP_PTR:
            {
                auto pointedType = std::make_unique<Type>();
                pointedType->type = std::move(type);

                type.emplace< Pointer >( std::move(pointedType), token );
            }
            break;
        default:
            done=true;
            break;
        }

        if( !done )
            tokensConsumed += provisionalyConsumed;
    } while(!done);

    RULE_LEAVE();
}

SourceLocation Type::getLocation() const {
    struct Visitor {
        SourceLocation operator()( std::monostate ) {
            ABORT()<<"Unreachable state";
        }

        SourceLocation operator()( const Identifier &id ) {
            return id.getLocation();
        }

        SourceLocation operator()( const Array &array ) {
            return array.token->location;
        }

        SourceLocation operator()( const Pointer &ptr ) {
            return ptr.token->location;
        }
    };

    return std::visit( Visitor{}, type );
}

} // namespace NonTerminals
