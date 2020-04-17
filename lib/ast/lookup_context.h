/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_LOOKUP_CONTEXT_H
#define AST_LOOKUP_CONTEXT_H

#include "ast/static_type.h"
#include "parser.h"
#include "slice.h"
#include "tokenizer.h"

#include <string>
#include <unordered_map>

namespace AST {

class LookupContext {
public:
    struct Symbol {
        const Tokenizer::Token *token;
        StaticTypeImpl::CPtr type;
        IdentifierId id;
    };

private:
    // Members
    std::unordered_map< std::string, StaticTypeImpl::CPtr > types;
    const LookupContext *parent = nullptr;

    std::unordered_map< String, Symbol > symbols;

public:
    explicit LookupContext(const LookupContext *parent = nullptr) :
        parent(parent)
    {}

    const LookupContext *getParent() const {
        return parent;
    }

    StaticTypeImpl::CPtr lookupType( String name ) const;
    StaticTypeImpl::CPtr lookupType( const NonTerminals::Type &type ) const;

    StaticTypeImpl::CPtr registerScalarType( ScalarTypeImpl &&type );

    void addFunctionPass1( const Tokenizer::Token *token );
    void addFunctionPass2( const Tokenizer::Token *token, StaticTypeImpl::CPtr type );

    const Symbol *lookupSymbol( String name ) const;
};

} // End namespace AST

#endif // AST_LOOKUP_CONTEXT_H
