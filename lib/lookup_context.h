#ifndef LOOKUP_CONTEXT_H
#define LOOKUP_CONTEXT_H

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

// A Context is anything that may contain further symbolic definitions. This may be a module, a struct, a function or even an
// anonymous block of code.
class LookupContext : private NoCopy {
    struct NamedObject {
        enum class Type { FunctionDef };
        std::variant< NonTerminals::FuncDef * > definition;
    };

    // My place in the world
    const LookupContext *parent = nullptr;

    std::unordered_map<String, NamedObject> symbols;

public:

    LookupContext(const LookupContext *parent) : parent(parent) {
    }

    void registerBuiltInType(const char *name);

    // Symbol table generation passes
    template <typename NT>
    void pass1(std::vector<NT> &definitions);
};

#endif // LOOKUP_CONTEXT_H
