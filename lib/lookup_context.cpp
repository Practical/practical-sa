/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * This file is file is copyright (C) 2018-2019 by its authors.
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "lookup_context.h"

#include "asserts.h"
#include "practical-errors.h"

#include <sstream>

using PracticalSemanticAnalyzer::IdentifierId;
using PracticalSemanticAnalyzer::TypeId;
using PracticalSemanticAnalyzer::ExpressionId;
using PracticalSemanticAnalyzer::StaticType;
using PracticalSemanticAnalyzer::SymbolRedefined;

static IdentifierId::Allocator<> idAllocator;
static TypeId::Allocator<> typeAllocator;

std::unordered_map<TypeId, const LookupContext::NamedType *> LookupContext::typeRepository;

LookupContext::NamedType::NamedType(const Tokenizer::Token *name, LookupContext::NamedType::Type type, size_t size) :
    _id(typeAllocator.allocate()), _size(size), _name(name), _type(type)
{
}

LookupContext::NamedType::NamedType( NamedType &&that )
    : _id(that._id), _size(that._size), _name(that._name), _type(that._type)
{
}

LookupContext::LocalVariable::LocalVariable(const Tokenizer::Token *name) : name(name), id(idAllocator.allocate())
{
}

LookupContext::LocalVariable::LocalVariable(const Tokenizer::Token *name, const Expression &lvalueExpression) :
    name(name),
    type( lvalueExpression.type ),
    expressionId( lvalueExpression.id ),
    id( idAllocator.allocate() )
{
}

LookupContext::Function::Function( const Tokenizer::Token *name ) : name(name) {}

LookupContext::LookupContext( const LookupContext *parent ) : parent(parent) {}

LookupContext::~LookupContext() {
    // Deregister our local symbols from the global registry
    for( const auto &i: types ) {
        typeRepository.erase( i.second.id() );
    }
}

LookupContext::NamedType *LookupContext::registerType( const Tokenizer::Token *name, NamedType::Type type, size_t size )
{
    ASSERT( size>0 || type==NamedType::Type::Void || type==NamedType::Type::Type );

    auto emplaceResultName = types.emplace( name->text, NamedType(name, type, size) );
    if( !emplaceResultName.second )
        throw SymbolRedefined( name->text, name->line, name->col );

    LookupContext::NamedType *namedType = &(emplaceResultName.first->second);
    auto emplaceResultId = typeRepository.emplace( namedType->id(), namedType );
    ASSERT( emplaceResultId.second ) <<
            "Type " << name->text << " has same ID " << namedType->id() << " as an existing type";

    return namedType;
}

LookupContext::NamedType *LookupContext::registerType(
            const Tokenizer::Token *name, NamedType::Type type, size_t size, FullRangeInt min, FullRangeInt max )
{
    NamedType *ret = registerType( name, type, size );
    ret->setRange( min, max );

    return ret;
}

LookupContext::Function *LookupContext::registerFunctionPass1( const Tokenizer::Token *name )
{
    auto emplaceResult = symbols.emplace( name->text, NamedObject( std::in_place_type<LookupContext::Function>, Function(name) ) );
    if( !emplaceResult.second )
        throw SymbolRedefined( name->text, name->line, name->col );

    return &std::get<LookupContext::Function>( emplaceResult.first->second );
}

void LookupContext::registerFunctionPass2(
        String name,
        PracticalSemanticAnalyzer::StaticType::Ptr &&returnType,
        std::vector<PracticalSemanticAnalyzer::ArgumentDeclaration> &&arguments)
{
    auto namedObject = symbols.find( name );
    ASSERT( namedObject != symbols.end() ) << "registerFunctionPass2 called with argument " << name << " that is not registered";
    Function *functionObject = std::get_if< Function >( & namedObject->second );
    ASSERT( functionObject != nullptr ) << "registerFunctionPass2 called with argument " << name << " which isn't a function";

    functionObject->returnType = std::move( returnType );
    ASSERT( functionObject->arguments.size()==0 ) <<
            "Function " << name << " already has " << functionObject->arguments.size() << " arguments defined at start of pass2";
    functionObject->arguments = std::move( arguments );
}

const LookupContext::LocalVariable *LookupContext::registerVariable( LocalVariable &&var ) {
    auto name = var.name->text;
    auto line = var.name->line;
    auto col = var.name->col;

    auto emplaceResult = symbols.emplace( name, std::move(var) );
    if( !emplaceResult.second )
        throw SymbolRedefined( name, line, col );

    return &std::get<LocalVariable>( emplaceResult.first->second );
}
const LookupContext::NamedObject *LookupContext::lookupIdentifier(String name) const {
    auto iterator = symbols.find(name);
    if( iterator!=symbols.end() )
        return &(iterator->second);

    if( parent==nullptr )
        return nullptr;

    return parent->lookupIdentifier(name);
}

const LookupContext::NamedType *LookupContext::lookupType(String name) const {
    auto iterator = types.find(name);
    if( iterator!=types.end() )
        return &(iterator->second);

    if( parent==nullptr )
        return nullptr;

    return parent->lookupType(name);
}

const LookupContext::NamedType *LookupContext::lookupType(TypeId id) {
    auto iterator = typeRepository.find(id);
    ASSERT( iterator!=typeRepository.end() ) << "Tried to look up type ID " << id << " which is no (longer?) defined.";
    return iterator->second;
}
