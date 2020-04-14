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
    ASSERT( prepared() )<<"codegen called without calling prepare first";
}

// Private methods
void AST::registerBuiltinTypes( BuiltinContextGen *ctxGen ) {
    ASSERT( !prepared() )<<"prepare called twice";

    builtinCtx.registerScalarType( ScalarImpl( "Void", 0, 1, ctxGen->registerVoidType() ) );
    builtinCtx.registerScalarType( ScalarImpl( "Bool", 1, 1, ctxGen->registerBoolType() ) );
    builtinCtx.registerScalarType( ScalarImpl( "S8", 8, 1, ctxGen->registerIntegerType( 8, 1, true ) ) );
    builtinCtx.registerScalarType( ScalarImpl( "S16", 16, 2, ctxGen->registerIntegerType( 16, 2, true ) ) );
    builtinCtx.registerScalarType( ScalarImpl( "S32", 32, 4, ctxGen->registerIntegerType( 32, 4, true ) ) );
    builtinCtx.registerScalarType( ScalarImpl( "S64", 64, 8, ctxGen->registerIntegerType( 64, 8, true ) ) );
    builtinCtx.registerScalarType( ScalarImpl( "U8", 8, 1, ctxGen->registerIntegerType( 8, 1, false ) ) );
    builtinCtx.registerScalarType( ScalarImpl( "U16", 16, 2, ctxGen->registerIntegerType( 16, 2, false ) ) );
    builtinCtx.registerScalarType( ScalarImpl( "U32", 32, 4, ctxGen->registerIntegerType( 32, 4, false ) ) );
    builtinCtx.registerScalarType( ScalarImpl( "U64", 64, 8, ctxGen->registerIntegerType( 64, 8, false ) ) );
}

} // End namespace AST
