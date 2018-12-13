#include "lookup_context.h"

#include "parser.h"
#include "practical-sa.h"

#include "typed.h"

static IdentifierId::Allocator<> idAllocator;

std::unordered_map<PracticalSemanticAnalyzer::IdentifierId, LookupContext::NamedObject*> LookupContext::identifierRepository;

template <typename T>
LookupContext::NamedObject::NamedObject( const T &arg )
    : definition(std::in_place_type<T>, arg)
{
    setId();
}

template <typename T>
LookupContext::NamedObject::NamedObject( T &&arg )
    : definition( std::in_place_type<T>, std::move(arg) )
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
    addSymbol( toSlice(name), NamedObject( BuiltInType() ) );
}

void LookupContext::addSymbol(String name, NamedObject &&definition) {
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

        IdentifierId operator()( const NonTerminals::FuncDef *func ) {
            return func->id;
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

        void operator()( NonTerminals::FuncDef *func ) {
            func->id = id;
        }
    };

    std::visit(Visitor(id), definition);
}

template <typename NT>
void LookupContext::symbolsPass1(std::vector< NT > &definitions) {
    for( auto &symbol : definitions ) {
        LookupContext::NamedObject def(&symbol);

        auto previousDefinition = symbols.find(symbol.name());
        if( previousDefinition == symbols.end() ) {
            addSymbol(symbol.name(), std::move(def));
        } else {
            // XXX Overloads not yet implemented
            assert(false);
        }
    }
}

void LookupContext::symbolsPass2(std::vector< NonTerminals::FuncDef > &definitions) {
    for( auto &funcDef : definitions ) {
        // Resolve types in decleration
        funcDef.symbolsPass2(this);
    }
}

// Since we're defining the template inside a cpp file, we need to explicitly instantiate the function
template void LookupContext::symbolsPass1<NonTerminals::FuncDef>(std::vector<NonTerminals::FuncDef> &);
