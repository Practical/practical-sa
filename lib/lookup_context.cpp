#include "lookup_context.h"

#include "parser.h"
#include "practical-sa.h"

#include "typed.h"

void LookupContext::registerBuiltInType(const char *name) {
}

template <typename NT>
void LookupContext::pass1(std::vector< NT > &definitions) {
    for( auto &symbol : definitions ) {
        LookupContext::NamedObject def;
        def.definition = &symbol;

        auto previousDefinition = symbols.find(symbol.name());
        if( previousDefinition == symbols.end() ) {
            symbols[symbol.name()] = def;
            // safenew doesn't work here, because we're invoking a private constructor. Thankfully, it is also not needed, as
            // emplace will invoke the unique_ptr constructor that accepts a naked pointer.
        } else {
            // XXX Overloads not yet implemented
            assert(false);
        }
    }
}

// Since we're defining the template inside a cpp file, we need to explicitly instantiate the function
template void LookupContext::pass1<NonTerminals::FuncDef>(std::vector<NonTerminals::FuncDef> &);
