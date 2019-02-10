#include "lookup_context.h"

#include "ast_nodes.h"
#include "practical-sa.h"

#include "typed.h"

#include <sstream>

static IdentifierId::Allocator<> idAllocator;

std::unordered_map<PracticalSemanticAnalyzer::IdentifierId, LookupContext::NamedObject*> LookupContext::identifierRepository;

SymbolRedefined::SymbolRedefined(String symbol, size_t line, size_t col)
    : compile_error(line, col)
{
    std::stringstream buf;

    buf<<"Symbol "<<symbol<<" redefined";

    setMsg( buf.str().c_str() );
}

VariableDef::VariableDef(const Tokenizer::Token *name, AST::Type &&type, ExpressionId lvalueId)
    : name(name), type( std::move(type).removeType()), lvalueId(lvalueId)
{
}

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

LookupContext::NamedObject::NamedObject( VariableDef &&definition ) : definition( std::move(definition) )
{
    setId();
}

LookupContext::~LookupContext() {
    // Deregister our local symbols from the global registry
    for( const auto &i: symbols ) {
        identifierRepository.erase( i.second.getId() );
    }
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

const VariableDef *LookupContext::addVariable(const Tokenizer::Token *name, AST::Type &&type, ExpressionId lvalueId) {
    auto iteratorPair = symbols.find(name->text);
    if( iteratorPair!=symbols.end() ) {
        // TODO add info about previous definition
        throw SymbolRedefined(name->text, name->line, name->col);
    }

    // TODO check whether shadowing a higher scope same name variable
    auto insertionIterator = symbols.emplace( name->text, NamedObject( VariableDef(name, std::move(type), lvalueId) ) );
    ASSERT(insertionIterator.second) << "Trying to add variable \"" << name->text << "\" that is already present";

    NamedObject *namedResult = &insertionIterator.first->second;
    identifierRepository.insert( std::make_pair( namedResult->getId(), namedResult ) );

    return &std::get<VariableDef>(namedResult->definition);
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

        IdentifierId operator()( const VariableDef &varDef ) {
            return varDef.id;
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

        void operator()( VariableDef &varDef ) {
            varDef.id = id;
        }
    };

    std::visit(Visitor(id), definition);
}
