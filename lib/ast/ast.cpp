/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast.h"

namespace AST {

LookupContext AST::builtinCtx;

// Public methods
void AST::prepare( BuiltinContextGen *ctxGen ) {
    registerBuiltinTypes( ctxGen );
}

bool AST::prepared() {
    return builtinCtx.lookupType("Void") != StaticTypeImpl::CPtr();
}

void AST::codeGen( const NonTerminals::Module &module, PracticalSemanticAnalyzer::ModuleGen *codeGen ) {
}

// Private methods
void AST::registerBuiltinTypes( BuiltinContextGen *ctxGen ) {
    ASSERT( !prepared() )<<"prepare called twice";
}

} // End namespace AST
