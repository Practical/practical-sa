#include "ast/struct.h"

#include "ast/hash_modifiers.h"
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

void StructTypeImpl::calcHash() {
    ASSERT( _hash==0 );
    _hash = calcHashHelper( this );
}

size_t StructTypeImpl::calcHash( const StructTypeImpl *anchor ) const {
    if( anchor==this ) {
        return RecursiveStructHash;
    }

    return calcHashHelper(anchor);
}

void StructTypeImpl::getMangledName(std::ostringstream &formatter) const {
    ABORT()<<"TODO implement";
}

void StructTypeImpl::definitionPass1( const NonTerminals::StructDef &parserStruct ) {
    ASSERT( _context )<<"definitionPass1 called without initializing a context";
}

bool StructTypeImpl::definitionPass2(
        const StaticTypeImpl *containingType,
        const NonTerminals::StructDef &parserStruct,
        DelayedDefinitions &delayedDefs )
{
    ASSERT( _context )<<"definitionPass2 called without initializing a context";
    ASSERT( _size==0 );
    ASSERT( _alignment==0 );
    ASSERT( _members.size()==0 );

    bool incomplete = false;
    _members.reserve( parserStruct.variables.size() );
    for( auto &parserVar : parserStruct.variables ) {
        auto varType = _context->lookupType( parserVar.body.type );

        if( varType->sizeKnown() ) {
            _size = alignUp(_size, varType->getAlignment());
            _alignment = std::max( _alignment, varType->getAlignment() );
            _members.push_back( _context->addStructMember( parserVar.body.name.identifier, varType, _size ) );
            _size += varType->getSize();
        } else {
            auto inserter = delayedDefs.pending.emplace( &parserStruct, const_cast<LookupContext*>(_context->getParent()) );
            inserter.first->second.dependencies.emplace( varType->coreType()->getType() );
            delayedDefs
                    .reverseDependencies[ varType->getType() ]
                    .dependants
                    .emplace(inserter.first);

            incomplete = true;
        }
    }

    if( incomplete ) {
        _size = 0;
        _alignment = 0;
        _members.clear();

        return false;
    }

    if( _size==0 )
        _size=1;

    auto revDepIter = delayedDefs.reverseDependencies.find( this );
    if( revDepIter!=delayedDefs.reverseDependencies.end() ) {
        for( auto dependant : revDepIter->second.dependants ) {
            dependant->second.dependencies.erase( this );

            if( dependant->second.dependencies.empty() ) {
                delayedDefs.ready.emplace( dependant->first, dependant->second.context );

                delayedDefs.pending.erase( dependant );
            }
        }
    }

    return true;
}

size_t StructTypeImpl::calcHashHelper( const StructTypeImpl *anchor ) const {
    ASSERT( getSize()!=0 )<<"Tried to get hash of incomplete type "<<this;

    size_t result = FibonacciHashMultiplier - StructModifier;

    result += std::hash<String>()( getName() );

    for( size_t i=0; i<getNumMembers(); ++i ) {
        StaticType::Struct::MemberDescriptor member = getMember(i);

        result *= FibonacciHashMultiplier;
        result += std::hash<String>()( member.name );
        result *= FibonacciHashMultiplier;
        result += downCast( member.type )->calcHashInternal( anchor );
    }

    return result;
}

} // namespace AST
