/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "statement.h"

namespace AST {

Statement::Statement( const NonTerminals::Statement &parserStatement )
{
    ABORT()<<"TODO implement";
}

void Statement::codeGen( LookupContext &lookupCtx, PracticalSemanticAnalyzer::FunctionGen *functionGen ) const {
    ABORT()<<"TODO implement";
}

} // namespace AST
