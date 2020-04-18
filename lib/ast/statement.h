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

#include "ast/lookup_context.h"
#include "parser.h"

namespace AST {

class Statement {
public:
    explicit Statement( const NonTerminals::Statement &parserStatement );

    void codeGen( LookupContext &lookupCtx, PracticalSemanticAnalyzer::FunctionGen *functionGen ) const;
};

} // namespace AST

#endif // AST_STATEMENT_H
