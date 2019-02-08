#ifndef LOOKUP_CONTEXT_H
#define LOOKUP_CONTEXT_H

#include <practical-sa.h>

#include "asserts.h"
#include "nocopy.h"
#include "slice.h"

#include <memory>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

// We need to forward declare the non-terminals we point to
namespace AST {
    struct FuncDef;
}

struct BuiltInType : public PracticalSemanticAnalyzer::NamedType::BuiltIn {
    BuiltInType( const char *_name, Type _type, uint8_t _sizeInBits )
    {
        name = toSlice(_name);
        type = _type;
        sizeInBits = _sizeInBits;

        ASSERT( type!=Type::Invalid );
    }
};

// A Context is anything that may contain further symbolic definitions. This may be a module, a struct, a function or even an
// anonymous block of code.
class LookupContext : private NoCopy {
public:
    struct NamedObject {
        struct Type {
            enum { BuiltInType, FuncDef, END };
        };
        std::variant< ::BuiltInType, AST::FuncDef * > definition;

        NamedObject( const ::BuiltInType &type );
        NamedObject( AST::FuncDef *funcDef );

        PracticalSemanticAnalyzer::IdentifierId getId() const;

        bool isType() const {
            auto defIdx = definition.index();
            ASSERT( defIdx!=std::variant_npos ) << "Attempt to query an empty variant";
            ASSERT( defIdx < Type::END ) << "Variant out of range";
            switch( defIdx ) {
            case Type::BuiltInType:
                return true;
            case Type::FuncDef:
                return false;
            case Type::END:
                ABORT() << "Unreachable code";
            default:
                ABORT();
            }

            ABORT() << "Switch missed an option";
        }
    private:
        void setId();
    };

private:
    // My place in the world
    const LookupContext *parent = nullptr;

    std::unordered_map<String, NamedObject> symbols;
    static std::unordered_map<PracticalSemanticAnalyzer::IdentifierId, NamedObject*> identifierRepository; // TODO thread safety

public:
    LookupContext(const LookupContext *parent) : parent(parent) {
    }

    const NamedObject *getSymbol(String name) const;
    const NamedObject *getSymbol(PracticalSemanticAnalyzer::IdentifierId id) const;

    void registerBuiltInType(const BuiltInType &type);
    void addSymbolPass1(String name, NamedObject &&definition);

    static const NamedObject *lookupIdentifier(PracticalSemanticAnalyzer::IdentifierId id);
};

#endif // LOOKUP_CONTEXT_H
