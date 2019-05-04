/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * This file is file is copyright (C) 2018-2019 by its authors.
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "casts.h"

#include "asserts.h"
#include "ast.h"
#include "dummy_codegen_impl.h"
#include "lookup_context.h"
#include "practical-errors.h"

using namespace PracticalSemanticAnalyzer;

bool checkImplicitCastAllowed(
        const Expression &sourceExpression, ExpectedType destType, const Tokenizer::Token &expressionSource)
{
    try {
        codeGenCast( &dummyFunctionGen, sourceExpression, destType, expressionSource, true );
    } catch(CastNotAllowed &ex) {
        if( destType.mandatory )
            throw;

        return false;
    }

    return true;
}

static Expression codeGenCast_SignedIntSource(
        FunctionGen *codeGen, const Expression &sourceExpression, const NamedType *sourceType,
        const NamedType *destNamedType, StaticType::Ptr destStaticType, const Tokenizer::Token &expressionSource,
        bool implicitOnly )
{
#define REPORT_ERROR() throw CastNotAllowed(sourceExpression.type, destStaticType, implicitOnly, expressionSource.line, expressionSource.col)
    if( destNamedType->type()!=NamedType::Type::SignedInteger )
        REPORT_ERROR();

    Expression castResult{ StaticType::Ptr(destStaticType) };
    if( sourceType->size()>destNamedType->size() ) {
        if( implicitOnly )
            REPORT_ERROR();

        codeGen->truncateInteger( castResult.id, sourceExpression.id, sourceExpression.type, destStaticType );
    } else {
        codeGen->expandIntegerSigned( castResult.id, sourceExpression.id, sourceExpression.type, destStaticType );
    }

    return castResult;
#undef REPORT_ERROR
}

static Expression codeGenCast_UnsignedIntSource(
        FunctionGen *codeGen, const Expression &sourceExpression, const NamedType *sourceType,
        const NamedType *destNamedType, StaticType::Ptr destStaticType, const Tokenizer::Token &expressionSource,
        bool implicitOnly )
{
#define REPORT_ERROR() throw CastNotAllowed(sourceExpression.type, destStaticType, implicitOnly, expressionSource.line, expressionSource.col)
    Expression castResult{ StaticType::Ptr(destStaticType) };

    if( destNamedType->type()==NamedType::Type::SignedInteger ) {
        if( sourceType->size()>=destNamedType->size() ) {
            if( implicitOnly )
                REPORT_ERROR();

            codeGen->truncateInteger( castResult.id, sourceExpression.id, sourceExpression.type, destStaticType );
        } else {
            codeGen->expandIntegerUnsigned( castResult.id, sourceExpression.id, sourceExpression.type, destStaticType );
        }
    } else if( destNamedType->type()==NamedType::Type::UnsignedInteger ) {
        if( sourceType->size()>destNamedType->size() ) {
            if( implicitOnly )
                REPORT_ERROR();

            codeGen->truncateInteger( castResult.id, sourceExpression.id, sourceExpression.type, destStaticType );
        } else {
            codeGen->expandIntegerUnsigned( castResult.id, sourceExpression.id, sourceExpression.type, destStaticType );
        }
    } else {
        ABORT() << "TODO implement";
    }

    return castResult;
#undef REPORT_ERROR
}

Expression codeGenCast(
        FunctionGen *codeGen, const Expression &sourceExpression, ExpectedType destType,
        const Tokenizer::Token &expressionSource, bool implicitOnly )
{
    return codeGenCast( codeGen, sourceExpression, destType.type, expressionSource, implicitOnly );
}

Expression codeGenCast(
        FunctionGen *codeGen, const Expression &sourceExpression, StaticType::Ptr destType,
        const Tokenizer::Token &expressionSource, bool implicitOnly )
{
    // Fastpath
    if( sourceExpression.type==destType )
        return sourceExpression.duplicate();

    const NamedType *namedSource = LookupContext::lookupType( sourceExpression.type->getId() );
    const NamedType *namedDest = LookupContext::lookupType( destType->getId() );

    ASSERT( namedSource!=nullptr );
    ASSERT( namedDest!=nullptr );

    if( namedSource->type()==NamedType::Type::SignedInteger )
        return codeGenCast_SignedIntSource(
                codeGen, sourceExpression, namedSource, namedDest, destType, expressionSource, implicitOnly);

    if( namedSource->type()==NamedType::Type::UnsignedInteger ) {
        return codeGenCast_UnsignedIntSource(
                codeGen, sourceExpression, namedSource, namedDest, destType, expressionSource, implicitOnly);
    }

    ABORT() << "Unreachable code reached";
}
