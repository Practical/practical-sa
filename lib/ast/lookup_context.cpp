/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "lookup_context.h"

#include <asserts.h>

using namespace PracticalSemanticAnalyzer;

namespace AST {

StaticTypeImpl::CPtr LookupContext::lookupType( String name ) const {
    auto iter = types.find( sliceToString(name) );

    if( iter==types.end() ) {
        if( parent )
            return parent->lookupType(name);
        else
            return StaticTypeImpl::CPtr();
    }

    return iter->second;
}

StaticTypeImpl::CPtr LookupContext::lookupType( const NonTerminals::Type &type ) const {
    return lookupType( type.type.identifier->text );
}

StaticTypeImpl::CPtr LookupContext::registerScalarType( ScalarTypeImpl &&type ) {
    std::string name = sliceToString(type.getName());
    auto iter = types.emplace(
            name,
            StaticTypeImpl::allocate( std::move(type) ) );
    ASSERT( iter.second )<<"registerBuiltinType called on "<<iter.first->second<<" ("<<iter.first->first<<", "<<name<<") which is already registered";

    return iter.first->second;
}

void LookupContext::addFunctionPass1( const Tokenizer::Token *token ) {
    symbols.emplace(
            token->text,
            Symbol{ .token = token } );
}

void LookupContext::addFunctionPass2( const Tokenizer::Token *token, StaticTypeImpl::CPtr type ) {
    auto iter = symbols.find( token->text );
    ASSERT( iter!=symbols.end() )<<"addFunctionPass2 called for "<<token->text<<", but not for addFunctionPass1";

    iter->second.type = std::move(type);
}

const LookupContext::Symbol *LookupContext::lookupSymbol( String name ) const {
    auto iter = symbols.find(name);
    if( iter!=symbols.end() )
        return &iter->second;

    if( getParent()==nullptr )
        return nullptr;

    return getParent()->lookupSymbol( name );
}

} // End namespace AST
