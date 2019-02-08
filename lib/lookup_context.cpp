#include "lookup_context.h"

#include "ast_nodes.h"
#include "practical-sa.h"

#include "typed.h"

static IdentifierId::Allocator<> idAllocator;

std::unordered_map<PracticalSemanticAnalyzer::IdentifierId, LookupContext::NamedObject*> LookupContext::identifierRepository;

LookupContext::NamedObject::NamedObject( const BuiltInType &type )
    : definition(std::in_place_type<BuiltInType>, type)
{
    setId();
}

LookupContext::NamedObject::NamedObject( const AST::FuncDef *funcDef )
    : definition( std::in_place_type<const AST::FuncDef *>, funcDef )
{
    setId();
}

const LookupContext::NamedObject *LookupContext::getSymbol(String name) const {
    auto iterator = symbols.find(name);
    if( iterator!=symbols.end() )
        return &(iterator->second);

    if( parent==nullptr )
        return nullptr;

    return parent->getSymbol(name);
}

const LookupContext::NamedObject *LookupContext::getSymbol(PracticalSemanticAnalyzer::IdentifierId id) const {
    return identifierRepository.at(id);
}

void LookupContext::registerBuiltInType( const BuiltInType &type ) {
    addSymbolPass1( type.name, NamedObject( type ) );
}

void LookupContext::addSymbolPass1(String name, NamedObject &&definition) {
    IdentifierId id(definition.getId());

    auto emplaceResult = symbols.try_emplace( name, std::move(definition));
    ASSERT(emplaceResult.second) << "Trying to add symbol \"" << name << "\" that is already present";
    identifierRepository.insert( std::make_pair( id, &(*emplaceResult.first).second ) );
}

const LookupContext::NamedObject *LookupContext::lookupIdentifier(PracticalSemanticAnalyzer::IdentifierId id) {
    return identifierRepository.at(id);
}

IdentifierId LookupContext::NamedObject::getId() const {
    struct Visitor {
        IdentifierId operator()( std::monostate none ) {
            ABORT() << "Asked for ID of an empty NamedObject";
        }

        IdentifierId operator()( const BuiltInType &builtin ) {
            return builtin.id;
        }

        IdentifierId operator()( const AST::FuncDef *func ) {
            return func->getId();
        }
    };

    IdentifierId id = std::visit(Visitor(), definition);
    ASSERT( id != IdentifierId() ) << "id is uninitialized";

    return id;
}

void LookupContext::NamedObject::setId() {
    IdentifierId id( idAllocator.allocate() );

    struct Visitor {
        IdentifierId id;

        Visitor(IdentifierId _id) : id(_id) {}

        void operator()( std::monostate none ) {
            ABORT() << "Tried to set ID of an empty NamedObject";
        }

        void operator()( BuiltInType &builtin ) {
            builtin.id = id;
        }

        void operator()( const AST::FuncDef *func ) {
            const_cast<AST::FuncDef *>(func)->setId( id );
        }
    };

    std::visit(Visitor(id), definition);
}
