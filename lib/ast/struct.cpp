#include "ast/struct.h"

#include "ast/lookup_context.h"

namespace AST {

using namespace PracticalSemanticAnalyzer;

StructTypeImpl::StructTypeImpl(String name, const LookupContext *parentCtx) :
    _name( sliceToString(name) ),
    _context( new LookupContext(parentCtx) )
{}

StructTypeImpl::StructTypeImpl( StructTypeImpl &&that ) :
    StaticType::Struct( std::move(that) ),
    _name( std::move(that._name) ),
    _context( std::move(that._context) )
{
}

StructTypeImpl::~StructTypeImpl()
{}

String StructTypeImpl::getName() const {
    return _name;
}

size_t StructTypeImpl::getNumMembers() const {
    return _members.size();
}

StaticType::Struct::MemberDescriptor StructTypeImpl::getMember( size_t index ) const {
    ASSERT( index<getNumMembers() );

    StaticType::Struct::MemberDescriptor ret;
    ret.name = _members.at(index);
    ret.type = std::get<StructMember>( *_context->lookupIdentifier(ret.name) ).type;

    return ret;
}

size_t StructTypeImpl::getSize() const {
    return _size;
}

size_t StructTypeImpl::getAlignment() const {
    return _alignment;
}

void StructTypeImpl::getMangledName(std::ostringstream &formatter) const {
    ABORT()<<"TODO implement";
}

void StructTypeImpl::definitionPass1( const NonTerminals::StructDef &parserStruct ) {
    ASSERT( _context )<<"definitionPass1 called without initializing a context";
}

void StructTypeImpl::definitionPass2( const NonTerminals::StructDef &parserStruct ) {
    ASSERT( _context )<<"definitionPass2 called without initializing a context";
    ASSERT( _size==0 );
    ASSERT( _alignment==0 );
    ASSERT( _members.size()==0 );

    _alignment = 1;

    _members.reserve( parserStruct.variables.size() );
    for( auto &parserVar : parserStruct.variables ) {
        auto varType = _context->lookupType( parserVar.body.type );
        _size = alignUp(_size, varType->getAlignment());
        _alignment = std::max( _alignment, varType->getAlignment() );
        _members.push_back( _context->addStructMember( parserVar.body.name.identifier, varType, _size ) );
        _size += varType->getSize();
    }
}

} // namespace AST
