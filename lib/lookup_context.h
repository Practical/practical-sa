#ifndef LOOKUP_CONTEXT_H
#define LOOKUP_CONTEXT_H

#include <practical-sa.h>

#include "nocopy.h"
#include "slice.h"

#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

// We need to forward declare the non-terminals we point to
namespace NonTerminals {
    struct FuncDef;
}

struct BuiltInType {
    String name;
    enum class Type { SignedInt, UnsignedInt } type;
    uint8_t size;
    PracticalSemanticAnalyzer::IdentifierId id;
};

// A Context is anything that may contain further symbolic definitions. This may be a module, a struct, a function or even an
// anonymous block of code.
class LookupContext : private NoCopy {
    struct NamedObject {
        std::variant< BuiltInType, NonTerminals::FuncDef * > definition;

        template <typename T> NamedObject( const T &arg );
        template <typename T> NamedObject( T &&arg );

        PracticalSemanticAnalyzer::IdentifierId getId() const;
    private:
        void setId();
    };

    // My place in the world
    const LookupContext *parent = nullptr;

    std::unordered_map<String, NamedObject> symbols;
    static std::unordered_map<PracticalSemanticAnalyzer::IdentifierId, NamedObject*> identifierRepository; // TODO thread safety

public:

    LookupContext(const LookupContext *parent) : parent(parent) {
    }

    void registerBuiltInType(const char *name, BuiltInType::Type type, uint8_t size);
    void addSymbol(String name, NamedObject &&definition);

    // Symbol table generation passes
    template <typename NT>
    void pass1(std::vector<NT> &definitions);
};

#endif // LOOKUP_CONTEXT_H
