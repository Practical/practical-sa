/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "lookup_context.h"

#include "ast/expression.h"
#include "ast/mangle.h"

#include <asserts.h>

#include <practical-errors.h>

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

StaticTypeImpl::CPtr LookupContext::registerScalarType( ScalarTypeImpl &&type, ValueRangeBase::CPtr defaultValueRange ) {
    std::string name = sliceToString(type.getName());
    auto iter = types.emplace(
            name,
            StaticTypeImpl::allocate( std::move(type), std::move(defaultValueRange) ) );
    ASSERT( iter.second )<<"registerBuiltinType called on "<<iter.first->second<<" ("<<iter.first->first<<", "<<name<<") which is already registered";

    return iter.first->second;
}

void LookupContext::addFunctionPass1( const Tokenizer::Token *token ) {
    auto iter = symbols.find( token->text );

    Function *function = nullptr;
    if( iter!=symbols.end() ) {
        function = std::get_if<Function>( &iter->second );
        if( function==nullptr )
            throw pass1_error( "Function is trying to overload a variable", token->line, token->col );
            // More info: where variable was first declared
    } else {
        auto inserter = symbols.emplace( token->text, Function{} );
        function = &std::get<Function>(inserter.first->second);
    }

    function->overloads.emplace_back( token );
    function->firstPassOverloads.emplace( token, function->overloads.size()-1 );
}

void LookupContext::addFunctionPass2( const Tokenizer::Token *token, StaticTypeImpl::CPtr type ) {
    auto iter = symbols.find( token->text );
    ASSERT( iter!=symbols.end() )<<"addFunctionPass2 called for "<<token->text<<", but not for addFunctionPass1";
    Function *function = std::get_if<Function>( &iter->second );
    ASSERT( function!=nullptr );

    auto definitionIdx = function->firstPassOverloads.find( token );
    ASSERT( definitionIdx!=function->firstPassOverloads.end() );

    Function::Definition *definition = &function->overloads.at(definitionIdx->second);
    definition->mangledName = getFunctionMangledName( token->text, type );
    definition->type = std::move(type);
    definition->codeGen = globalFunctionCall;
}

const LookupContext::Identifier *LookupContext::lookupIdentifier( String name ) const {
    auto iter = symbols.find(name);
    if( iter==symbols.end() ) {
        if( getParent()==nullptr )
            return nullptr;

        return getParent()->lookupIdentifier( name );
    }

    return &iter->second;
}

void LookupContext::addLocalVar( const Tokenizer::Token *token, StaticTypeImpl::CPtr type, ExpressionId lvalue ) {
    auto iter = symbols.emplace( token->text, Variable(token, type, lvalue) );

    if( !iter.second ) {
        throw SymbolRedefined(token->text, token->line, token->col);
    }
}

void LookupContext::addCast(
        PracticalSemanticAnalyzer::StaticType::CPtr sourceType,
        PracticalSemanticAnalyzer::StaticType::CPtr destType,
        unsigned weight, CodeGenCast codeGenCast, bool implicitAllowed )
{
    ASSERT( getParent()==nullptr )<<"Non-builtin lookups not yet implemented";
    ASSERT( implicitAllowed )<<"TODO implement non-implicit casts";

    {
        auto &sourceTypeMap = typeConversionsFrom[sourceType];

        auto insertIterator = sourceTypeMap.emplace(
                std::piecewise_construct,
                std::tuple(destType),
                std::tuple(codeGenCast, weight) );
        ASSERT( insertIterator.second );
    }

    {
        auto &destTypeSet = typeConversionsTo[destType];

        auto insertIterator = destTypeSet.emplace( sourceType );
        ASSERT( insertIterator.second );
    }
}

const LookupContext::CastDescriptor *LookupContext::lookupCast(
        PracticalSemanticAnalyzer::StaticType::CPtr sourceType,
        PracticalSemanticAnalyzer::StaticType::CPtr destType,
        bool implicit ) const
{
    auto conversionIter = typeConversionsFrom.find( sourceType );
    if( conversionIter!=typeConversionsFrom.end() ) {
        auto conversionFuncIter = conversionIter->second.find( destType );
        if( conversionFuncIter!=conversionIter->second.end() ) {
            return &conversionFuncIter->second;
        }
    }

    if( getParent()==nullptr )
        return nullptr;

    return getParent()->lookupCast( sourceType, destType, implicit );
}

// Private methods
ExpressionId LookupContext::globalFunctionCall(
        Slice<Expression> arguments, const Function::Definition *definition,
        PracticalSemanticAnalyzer::FunctionGen *functionGen)
{
    ExpressionId resultId = Expression::allocateId();
    ExpressionId argumentExpressionIds[arguments.size()];
    for( unsigned argIdx=0; argIdx<arguments.size(); ++argIdx ) {
        argumentExpressionIds[argIdx] = arguments[argIdx].codeGen( functionGen );
    }

    functionGen->callFunctionDirect(
            resultId, definition->mangledName, Slice(argumentExpressionIds, arguments.size()),
            std::get<const StaticType::Function *>(definition->type->getType())->getReturnType() );

    return resultId;
}

} // End namespace AST
