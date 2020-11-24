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
#include "ast/pointers.h"

#include <asserts.h>

#include <practical/errors.h>

using namespace PracticalSemanticAnalyzer;

namespace AST {

StaticType::CPtr LookupContext::Function::Definition::returnType() const {
    auto functionType = std::get<const StaticType::Function *>( type->getType() );
    return functionType->getReturnType();
}

StaticTypeImpl::CPtr LookupContext::_genericFunctionType =
    StaticTypeImpl::allocate( FunctionTypeImpl( nullptr, {} ) );
ValueRangeBase::CPtr LookupContext::_genericFunctionRange =
    new PointerValueRange( nullptr, BoolValueRange(false, false) );

StaticTypeImpl::CPtr LookupContext::lookupType( String name, const SourceLocation &location ) const {
    auto iter = types.find( sliceToString(name) );

    if( iter==types.end() ) {
        if( parent )
            return parent->lookupType(name, location);
        else
            throw SymbolNotFound( name, location );
    }

    return iter->second;
}

StaticTypeImpl::CPtr LookupContext::lookupType( String name ) const {
    ASSERT( ! parent )<<"Lookup type without location only valid on built-in context";

    auto iter = types.find( sliceToString(name) );
    ASSERT( iter != types.end() )<<"Lookup failed on built-in type "<<name;

    return iter->second;
}

StaticTypeImpl::CPtr LookupContext::lookupType( const NonTerminals::Type &type ) const {
    struct Visitor {
        const LookupContext *_this;

        StaticTypeImpl::CPtr operator()( std::monostate ) {
            ABORT()<<"Unreachable state";
        }


        StaticTypeImpl::CPtr operator()( const NonTerminals::Identifier &id ) {
            return _this->lookupType( id.identifier->text, id.identifier->location );
        }

        StaticTypeImpl::CPtr operator()( const NonTerminals::Type::Pointer &ptr )
        {
            auto ret = _this->lookupType( *ptr.pointed );

            if( !ret )
                return StaticTypeImpl::CPtr();

            return StaticTypeImpl::allocate( PointerTypeImpl( std::move(ret) ) );
        }
    };

    return std::visit( Visitor( { ._this = this } ), type.type );
}

StaticTypeImpl::CPtr LookupContext::lookupType( const NonTerminals::TransientType &type ) const {
    auto ret = lookupType( type.type );
    if( type.ref )
        ret = downCast( ret->setFlags( StaticType::Flags::Reference ) );

    return ret;
}

StaticTypeImpl::CPtr LookupContext::registerScalarType( ScalarTypeImpl &&type, ValueRangeBase::CPtr defaultValueRange ) {
    std::string name = sliceToString(type.getName());
    auto iter = types.emplace(
            name,
            StaticTypeImpl::allocate( std::move(type), std::move(defaultValueRange) ) );
    ASSERT( iter.second )<<"registerBuiltinType called on "<<iter.first->second<<" ("<<iter.first->first<<", "<<name<<") which is already registered";

    return iter.first->second;
}

void LookupContext::addBuiltinFunction(
        const std::string &name, StaticTypeImpl::CPtr returnType, Slice<const StaticTypeImpl::CPtr> argumentTypes,
        Function::Definition::CodeGenProto *codeGen, Function::Definition::VrpProto *calcVrp)
{
    auto iter = symbols.find( name );

    Function *function = nullptr;
    if( iter!=symbols.end() ) {
        function = std::get_if<Function>( &iter->second );
        ASSERT( function!=nullptr );
    } else {
        auto inserter = symbols.emplace( name, Function{} );
        function = &std::get<Function>(inserter.first->second);
    }

    Function::Definition &definition = function->overloads.emplace_back(name);
    definition.type =
                StaticTypeImpl::allocate(
                    FunctionTypeImpl(
                        std::move(returnType),
                        std::vector(argumentTypes.begin(), argumentTypes.end())
                    )
                );
    definition.codeGen = codeGen;
    definition.calcVrp = calcVrp;
}

void LookupContext::addFunctionPass1( const Tokenizer::Token *token ) {
    auto iter = symbols.find( token->text );

    Function *function = nullptr;
    if( iter!=symbols.end() ) {
        function = std::get_if<Function>( &iter->second );
        if( function==nullptr )
            throw pass1_error( "Function is trying to overload a variable", token->location );
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

StaticTypeImpl::CPtr LookupContext::genericFunctionType() {
    return _genericFunctionType;
}

ValueRangeBase::CPtr LookupContext::genericFunctionRange() {
    return _genericFunctionRange;
}

void LookupContext::addLocalVar( const Tokenizer::Token *token, StaticTypeImpl::CPtr type, ExpressionId lvalue ) {
    auto iter = symbols.emplace( token->text, Variable(token, type, lvalue) );

    if( !iter.second ) {
        throw SymbolRedefined(token->text, token->location);
    }
}

void LookupContext::addCast(
        StaticTypeImpl::CPtr sourceType,
        StaticTypeImpl::CPtr destType,
        unsigned weight, CodeGenCast codeGenCast, ValueRangeCast calcVrp,
        CastDescriptor::ImplicitCastAllowed whenPossible )
{
    ASSERT( getParent()==nullptr )<<"Non-builtin lookups not yet implemented";

    {
        auto &sourceTypeMap = typeConversionsFrom[sourceType];

        auto insertIterator = sourceTypeMap.emplace(
                std::piecewise_construct,
                std::tuple(destType),
                std::tuple(sourceType, destType, codeGenCast, calcVrp, weight, whenPossible) );
        ASSERT( insertIterator.second );
    }

    {
        auto &destTypeSet = typeConversionsTo[destType];

        auto insertIterator = destTypeSet.emplace( sourceType );
        ASSERT( insertIterator.second );
    }
}

void LookupContext::CastsList::sort() {
    struct Comparer {
        bool operator()( const CastDescriptor *lhs, const CastDescriptor *rhs ) {
            return lhs->weight < rhs->weight;
        }
    };

    std::sort( casts.begin(), casts.end(), Comparer() );
}

const LookupContext::CastDescriptor *LookupContext::lookupCast(
        PracticalSemanticAnalyzer::StaticType::CPtr sourceType,
        PracticalSemanticAnalyzer::StaticType::CPtr destType
    ) const
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

    return getParent()->lookupCast( sourceType, destType );
}

LookupContext::CastsList LookupContext::allCastsTo(
        PracticalSemanticAnalyzer::StaticType::CPtr destType ) const
{
    CastsList ret;

    for( const LookupContext *_this = this; _this!=nullptr; _this = _this->parent ) {
        auto destTypeIter = _this->typeConversionsTo.find(destType);

        if( destTypeIter != _this->typeConversionsTo.end() ) {
            for( auto sourceType : destTypeIter->second ) {
                ret.casts.emplace_back(
                        & _this->typeConversionsFrom.at( sourceType ).at( destType )
                    );
            }
        }
    }

    ret.sort();

    return ret;
}

LookupContext::CastsList LookupContext::allCastsFrom(
        PracticalSemanticAnalyzer::StaticType::CPtr sourceType) const
{
    CastsList ret;

    for( const LookupContext *_this = this; _this!=nullptr; _this = _this->parent ) {
        auto sourceTypeIter = _this->typeConversionsFrom.find(sourceType);

        if( sourceTypeIter != _this->typeConversionsFrom.end() ) {
            for( auto &destTypeIter : sourceTypeIter->second ) {
                ret.casts.emplace_back( & destTypeIter.second );
            }
        }
    }

    ret.sort();

    return ret;
}

// Private methods
ExpressionId LookupContext::globalFunctionCall(
        Slice<const Expression> arguments, const Function::Definition *definition,
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
