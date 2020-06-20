/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_STATEMENT_H
#define AST_STATEMENT_H

#include "ast/expression.h"
#include "ast/lookup_context.h"
#include "ast/variable_definition.h"
#include "parser.h"

namespace AST {

class CompoundStatement;

class Statement {
    const NonTerminals::Statement &parserStatement;
    std::variant<std::monostate, Expression, VariableDefinition, std::unique_ptr<CompoundStatement>> underlyingStatement;

public:
    explicit Statement( const NonTerminals::Statement &parserStatement );
    Statement( const Statement &rhs ); // Declared only. Should never be called
    ~Statement();

    void buildAST( LookupContext &lookupCtx );
    void codeGen( const LookupContext &lookupCtx, PracticalSemanticAnalyzer::FunctionGen *functionGen ) const;
};

} // namespace AST

#endif // AST_STATEMENT_H
