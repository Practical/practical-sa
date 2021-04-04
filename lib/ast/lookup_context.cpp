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

static std::unordered_map< String, LookupContext::AbiType > abiLookupTable {
    { "Practical", LookupContext::AbiType::Practical },
    { "C", LookupContext::AbiType::C }
};

StaticType::CPtr LookupContext::Function::Definition::returnType() const {
    auto functionType = std::get<const StaticType::Function *>( type->getType() );
    return functionType->getReturnType();
}

StaticTypeImpl::CPtr LookupContext::_genericFunctionType =
    StaticTypeImpl::allocate( FunctionTypeImpl( nullptr, {} ) );
ValueRangeBase::CPtr LookupContext::_genericFunctionRange =
    new PointerValueRange( nullptr, BoolValueRange(false, false) );

StaticTypeImpl::CPtr LookupContext::lookupType( String name, const SourceLocation &location ) const {
    auto iter = _types.find( sliceToString(name) );

    if( iter==_types.end() ) {
        if( _parent )
            return _parent->lookupType(name, location);
        else
            throw SymbolNotFound( name, location );
    }

    return iter->second;
}

StaticTypeImpl::CPtr LookupContext::lookupType( String name ) const {
    ASSERT( ! _parent )<<"Lookup type without location only valid on built-in context";

    auto iter = _types.find( sliceToString(name) );
    ASSERT( iter != _types.end() )<<"Lookup failed on built-in type "<<name;

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

        StaticTypeImpl::CPtr operator()( const NonTerminals::Type::Array &array )
        {
            auto ret = _this->lookupType( *array.elementType );

            if( !ret )
                return StaticTypeImpl::CPtr();

            return StaticTypeImpl::allocate( ArrayTypeImpl( std::move(ret), array.dimension.value ) );
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
    auto iter = _types.emplace(
            name,
            StaticTypeImpl::allocate( std::move(type), std::move(defaultValueRange) ) );
    ASSERT( iter.second )<<"registerBuiltinType called on "<<iter.first->second<<" ("<<iter.first->first<<", "<<name<<") which is already registered";

    return iter.first->second;
}

void LookupContext::addBuiltinFunction(
        const std::string &name, StaticTypeImpl::CPtr returnType, Slice<const StaticTypeImpl::CPtr> argumentTypes,
        Function::Definition::CodeGenProto *codeGen, Function::Definition::VrpProto *calcVrp)
{
    auto iter = _symbols.find( name );

    Function *function = nullptr;
    if( iter!=_symbols.end() ) {
        function = std::get_if<Function>( &iter->second );
        ASSERT( function!=nullptr );
    } else {
        auto inserter = _symbols.emplace( name, Function{} );
        function = &std::get<Function>(inserter.first->second);
    }

    StaticTypeImpl::CPtr type = StaticTypeImpl::allocate(
                    FunctionTypeImpl(
                        std::move(returnType),
                        std::vector(argumentTypes.begin(), argumentTypes.end())
                    )
                );
    auto insertIter = function->overloads.emplace(
            std::piecewise_construct,
            std::make_tuple( type ),
            std::make_tuple( nullptr, name ) );
    ASSERT( insertIter.second )<<"Builtin function "<<name<<" "<<*type<<" added twice";

    Function::Definition &definition = insertIter.first->second;
    definition.type = type;
    definition.codeGen = codeGen;
    definition.calcVrp = calcVrp;
}

std::ostream &operator<<( std::ostream &out, LookupContext::AbiType abi ) {

#define CASE(a) case LookupContext::AbiType::a:\
    return out << #a

    switch(abi) {
        CASE(Practical);
        CASE(C);
    }

#undef CASE

    return out<<"AbiType("<< static_cast<int>(abi) << ")";
}

void LookupContext::addFunctionDeclarationPass1( const Tokenizer::Token *token ) {
    addFunctionDefinitionPass1(token);
}

void LookupContext::addFunctionDefinitionPass1( const Tokenizer::Token *token ) {
    auto iter = _symbols.find( token->text );

    Function *function = nullptr;
    if( iter!=_symbols.end() ) {
        function = std::get_if<Function>( &iter->second );
        if( function==nullptr )
            throw pass1_error( "Function is trying to overload a variable", token->location );
            // More info: where variable was first declared
    } else {
        auto inserter = _symbols.emplace( token->text, Function{} );
        function = &std::get<Function>(inserter.first->second);
    }

    function->firstPassOverloads.emplace( token, function->overloads.end() );
}

void LookupContext::addStructPass1( const NonTerminals::StructDef &def ) {
    auto inserter = _typesUnderConstruction.emplace(
            sliceToString(def.identifier.identifier->text),
            StaticTypeImpl::allocate( StructTypeImpl{ def.identifier.identifier->text, this } ) );
    if( !inserter.second )
        throw pass1_error( "Type redefinition", def.identifier.identifier->location );

    StructTypeImpl *strct = inserter.first->second->getMutableStruct();

    auto inserter2 = _types.emplace( sliceToString(def.identifier.identifier->text), inserter.first->second );
    if( !inserter2.second )
        throw pass1_error( "Type redefinition", def.identifier.identifier->location );

    strct->definitionPass1( def );
}

void LookupContext::addStructPass2( const NonTerminals::StructDef &def ) {
    auto iter = _typesUnderConstruction.find( sliceToString(def.identifier.identifier->text) );
    ASSERT( iter!=_typesUnderConstruction.end() );
    StructTypeImpl *strct = iter->second->getMutableStruct();

    strct->definitionPass2( def );

    iter->second->completeConstruction();
    _typesUnderConstruction.erase(iter);
}

void LookupContext::addFunctionDeclarationPass2(
        const Tokenizer::Token *token, StaticTypeImpl::CPtr type, AbiType abi )
{
    addFunctionPass2( token, type, abi, false );
}

void LookupContext::addFunctionDefinitionPass2(
        const Tokenizer::Token *token, StaticTypeImpl::CPtr type, AbiType abi )
{
    Function::Definition &definition = addFunctionPass2( token, type, abi, true );

    if( !definition.declarationOnly ) {
        throw MultipleDefinitions( token->location );
    }

    definition.declarationOnly = false;
}

void LookupContext::declareFunctions( PracticalSemanticAnalyzer::ModuleGen *moduleGen ) const
{
    for( auto &symbol : _symbols ) {
        const Function *function = std::get_if<Function>( &symbol.second );
        if( function==nullptr )
            continue;

        for( const auto &overload : function->overloads ) {
            moduleGen->declareIdentifier( overload.second.token->text, overload.second.mangledName, overload.first );
        }
    }
}

LookupContext::AbiType LookupContext::parseAbiString( String abiString, const SourceLocation &location ) {
    auto abiType = abiLookupTable.find( abiString );
    if( abiType==abiLookupTable.end() ) {
        throw UnidentifiedAbiString(location);
    }

    return abiType->second;
}

const LookupContext::Identifier *LookupContext::lookupIdentifier( String name ) const {
    auto iter = _symbols.find(name);
    if( iter==_symbols.end() ) {
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

void LookupContext::addLocalVar( const Tokenizer::Token *token, StaticTypeImpl::CPtr type, ExpressionId lvalue )
{
    auto iter = _symbols.emplace( token->text, Variable(token, type, lvalue) );

    if( !iter.second ) {
        throw SymbolRedefined(token->text, token->location);
    }
}

void LookupContext::addStructMember( const Tokenizer::Token *token, StaticTypeImpl::CPtr type, size_t offset )
{
    auto iter = _symbols.emplace( token->text, StructMember(token, type, offset) );

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
        auto &sourceTypeMap = _typeConversionsFrom[sourceType];

        auto insertIterator = sourceTypeMap.emplace(
                std::piecewise_construct,
                std::tuple(destType),
                std::tuple(sourceType, destType, codeGenCast, calcVrp, weight, whenPossible) );
        ASSERT( insertIterator.second );
    }

    {
        auto &destTypeSet = _typeConversionsTo[destType];

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
    auto conversionIter = _typeConversionsFrom.find( sourceType );
    if( conversionIter!=_typeConversionsFrom.end() ) {
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

    for( const LookupContext *_this = this; _this!=nullptr; _this = _this->_parent ) {
        auto destTypeIter = _this->_typeConversionsTo.find(destType);

        if( destTypeIter != _this->_typeConversionsTo.end() ) {
            for( auto sourceType : destTypeIter->second ) {
                ret.casts.emplace_back(
                        & _this->_typeConversionsFrom.at( sourceType ).at( destType )
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

    for( const LookupContext *_this = this; _this!=nullptr; _this = _this->_parent ) {
        auto sourceTypeIter = _this->_typeConversionsFrom.find(sourceType);

        if( sourceTypeIter != _this->_typeConversionsFrom.end() ) {
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

LookupContext::Function::Definition &LookupContext::addFunctionPass2(
        const Tokenizer::Token *token, StaticTypeImpl::CPtr type, AbiType abi, bool isDefinition )
{
    auto iter = _symbols.find( token->text );
    ASSERT( iter!=_symbols.end() )<<"addFunctionPass2 called for "<<token->text<<" without 1st pass";
    Function *function = std::get_if<Function>( &iter->second );
    ASSERT( function!=nullptr );

    auto insertIter = function->overloads.emplace(
            std::piecewise_construct,
            std::make_tuple( type ),
            std::make_tuple( token, sliceToString(token->text) ) );
    Function::Definition &definition = insertIter.first->second;

    if( ! insertIter.second && ( !definition.declarationOnly || !isDefinition ) ) {
        // Function already declared/defined
        return definition;
    }

    auto firstPassIter = function->firstPassOverloads.find(token);
    if( firstPassIter != function->firstPassOverloads.end() ) {
        ASSERT( firstPassIter->second == function->overloads.end() );
        firstPassIter->second = insertIter.first;
    }

    definition.mangledName = getFunctionMangledName( token->text, type, abi );
    definition.type = std::move(type);
    definition.codeGen = globalFunctionCall;

    return definition;
}

} // End namespace AST
