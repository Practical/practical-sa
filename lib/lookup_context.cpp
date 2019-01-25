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

LookupContext::NamedObject::NamedObject( AST::FuncDef *funcDef )
    : definition( std::in_place_type<AST::FuncDef *>, funcDef )
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

void LookupContext::registerBuiltInType(const char *name, BuiltInType::Type type, uint8_t size) {
    addSymbolPass1( toSlice(name), NamedObject( BuiltInType() ) );
}

void LookupContext::addSymbolPass1(String name, NamedObject &&definition) {
    IdentifierId id(definition.getId());

    auto emplaceResult = symbols.try_emplace( name, std::move(definition));
    assert(emplaceResult.second); // Trying to add a symbol that is already present
    identifierRepository.insert( std::make_pair( id, &(*emplaceResult.first).second ) );
}

IdentifierId LookupContext::NamedObject::getId() const {
    struct Visitor {
        IdentifierId operator()( const BuiltInType &builtin ) {
            return builtin.id;
        }

        IdentifierId operator()( const AST::FuncDef *func ) {
            return func->getId();
        }
    };

    IdentifierId id = std::visit(Visitor(), definition);
    assert( id != IdentifierId() ); // id is uninitialized

    return id;
}

void LookupContext::NamedObject::setId() {
    IdentifierId id( idAllocator.allocate() );

    struct Visitor {
        IdentifierId id;

        Visitor(IdentifierId _id) : id(_id) {}

        void operator()( BuiltInType &builtin ) {
            builtin.id = id;
        }

        void operator()( AST::FuncDef *func ) {
            func->setId( id );
        }
    };

    std::visit(Visitor(id), definition);
}
