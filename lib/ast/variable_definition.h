/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_VARIABLE_DEFINITION_H
#define AST_VARIABLE_DEFINITION_H

#include "ast/expression.h"
#include "ast/lookup_context.h"
#include "parser.h"

#include <practical/practical.h>

#include <optional>

namespace AST {

class VariableDefinition {
    const NonTerminals::VariableDefinition &parserVarDef;
    std::optional<Expression> initValue;

public:
    explicit VariableDefinition(const NonTerminals::VariableDefinition &parserVarDef);

    void buildAST( LookupContext &lookupCtx );
    void codeGen( const LookupContext &lookupCtx, PracticalSemanticAnalyzer::FunctionGen *functionGen ) const;
};

} // namespace AST

#endif // AST_VARIABLE_DEFINITION_H
