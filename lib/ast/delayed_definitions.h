/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2021 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_DELAYED_DEFINITIONS_H
#define AST_DELAYED_DEFINITIONS_H

#include "parser/struct.h"

#include <practical/practical.h>

#include <unordered_map>
#include <unordered_set>

namespace AST {

class LookupContext;

struct PendingType {
    LookupContext *context;
    std::unordered_set<PracticalSemanticAnalyzer::StaticType::Types> dependencies;

    explicit PendingType( LookupContext *ctx ) :
        context(ctx)
    {}
};

using ReadyTypes = std::unordered_map<const NonTerminals::StructDef *, LookupContext *>;
using PendingTypes = std::unordered_map<const NonTerminals::StructDef *, PendingType>;

} // namespace AST

namespace std {
    template<>
    class hash<AST::PendingTypes::iterator> {
    public:
        size_t operator()(AST::PendingTypes::iterator iter) const noexcept {
            return hash< std::add_pointer_t<decltype(*iter)> >()( &*iter );
        }
    };
}


namespace AST {

struct ReverseDependency {
    std::unordered_set<PendingTypes::iterator> dependants;
};

using ReverseDependencies =
    std::unordered_map<PracticalSemanticAnalyzer::StaticType::Types, ReverseDependency>;

struct DelayedDefinitions {
    ReadyTypes ready;
    PendingTypes pending;
    ReverseDependencies reverseDependencies;
};

} // namespace AST

#endif // AST_DELAYED_DEFINITIONS_H
