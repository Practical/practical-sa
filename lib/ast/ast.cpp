/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast.h"

#include "ast/bool_value_range.h"
#include "ast/signed_int_value_range.h"
#include "ast/unsigned_int_value_range.h"

namespace AST {

LookupContext AST::builtinCtx;

// Public methods
void AST::prepare( BuiltinContextGen *ctxGen ) {
    registerBuiltinTypes( ctxGen );
}

bool AST::prepared() {
    return builtinCtx.lookupType("Void") != StaticTypeImpl::CPtr();
}

void AST::codeGen( const NonTerminals::Module &parserModule, PracticalSemanticAnalyzer::ModuleGen *codeGen ) {
    ASSERT( prepared() )<<"codegen called without calling prepare first";
    module = new Module( parserModule, builtinCtx );

    module->symbolsPass1();
    module->symbolsPass2();

    module->codeGen( codeGen );
}

// Private methods
void AST::registerBuiltinTypes( BuiltinContextGen *ctxGen ) {
    ASSERT( !prepared() )<<"prepare called twice";

    builtinCtx.registerScalarType(
            ScalarTypeImpl( "Void", 0, 1, ScalarTypeImpl::Type::Void, ctxGen->registerVoidType() ),
            nullptr );
    builtinCtx.registerScalarType(
            ScalarTypeImpl( "Bool", 1, 1, ScalarTypeImpl::Type::Bool, ctxGen->registerBoolType() ),
            BoolValueRange::allocate() );
    builtinCtx.registerScalarType(
            ScalarTypeImpl( "S8", 8, 1, ScalarTypeImpl::Type::SignedInt, ctxGen->registerIntegerType( 8, 1, true ) ),
            SignedIntValueRange::allocate<int8_t>() );
    builtinCtx.registerScalarType(
            ScalarTypeImpl( "S16", 16, 2, ScalarTypeImpl::Type::SignedInt, ctxGen->registerIntegerType( 16, 2, true ) ),
            SignedIntValueRange::allocate<int16_t>() );
    builtinCtx.registerScalarType(
            ScalarTypeImpl( "S32", 32, 4, ScalarTypeImpl::Type::SignedInt, ctxGen->registerIntegerType( 32, 4, true ) ),
            SignedIntValueRange::allocate<int32_t>() );
    builtinCtx.registerScalarType(
            ScalarTypeImpl( "S64", 64, 8, ScalarTypeImpl::Type::SignedInt, ctxGen->registerIntegerType( 64, 8, true ) ),
            SignedIntValueRange::allocate<int64_t>() );
    builtinCtx.registerScalarType(
            ScalarTypeImpl( "U8", 8, 1, ScalarTypeImpl::Type::UnsignedInt, ctxGen->registerIntegerType( 8, 1, false ) ),
            UnsignedIntValueRange::allocate<uint8_t>() );
    builtinCtx.registerScalarType(
            ScalarTypeImpl( "U16", 16, 2, ScalarTypeImpl::Type::UnsignedInt, ctxGen->registerIntegerType( 16, 2, false ) ),
            UnsignedIntValueRange::allocate<uint16_t>() );
    builtinCtx.registerScalarType(
            ScalarTypeImpl( "U32", 32, 4, ScalarTypeImpl::Type::UnsignedInt, ctxGen->registerIntegerType( 32, 4, false ) ),
            UnsignedIntValueRange::allocate<uint32_t>() );
    builtinCtx.registerScalarType(
            ScalarTypeImpl( "U64", 64, 8, ScalarTypeImpl::Type::UnsignedInt, ctxGen->registerIntegerType( 64, 8, false ) ),
            UnsignedIntValueRange::allocate<uint64_t>() );
}

} // End namespace AST
