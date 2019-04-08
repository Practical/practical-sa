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
        ExpressionId id, StaticType::Ptr sourceType, ExpectedType destType, const Tokenizer::Token &expressionSource)
{
    try {
        codeGenCast( &dummyFunctionGen, id, sourceType, destType, expressionSource, true );
    } catch(CastNotAllowed &ex) {
        if( destType.mandatory )
            throw;

        return false;
    }

    return true;
}

static ExpressionId codeGenCast_SignedIntSource(
        FunctionGen *codeGen, ExpressionId sourceExpression, const NamedType *sourceType, const NamedType *destType,
        const Tokenizer::Token &expressionSource, bool implicitOnly )
{
#define REPORT_ERROR() throw CastNotAllowed(StaticType::allocate(sourceType->id()), StaticType::allocate(destType->id()), implicitOnly, expressionSource.line, expressionSource.col)
    if( destType->type()!=NamedType::Type::SignedInteger )
        REPORT_ERROR();

    ExpressionId castResult = expressionIdAllocator.allocate();
    if( sourceType->size()>destType->size() ) {
        if( implicitOnly )
            REPORT_ERROR();

        codeGen->truncateInteger(
                castResult, sourceExpression, StaticType::allocate(sourceType->id()),
                StaticType::allocate(destType->id()) );
    } else {
        codeGen->expandIntegerSigned(
                castResult, sourceExpression, StaticType::allocate(sourceType->id()),
                StaticType::allocate(destType->id()) );
    }

    return castResult;
#undef REPORT_ERROR
}

static ExpressionId codeGenCast_UnsignedIntSource(
        FunctionGen *codeGen, ExpressionId sourceExpression, const NamedType *sourceType, const NamedType *destType,
        const Tokenizer::Token &expressionSource, bool implicitOnly )
{
#define REPORT_ERROR() throw CastNotAllowed(StaticType::allocate(sourceType->id()), StaticType::allocate(destType->id()), implicitOnly, expressionSource.line, expressionSource.col)
    ExpressionId castResult = expressionIdAllocator.allocate();

    if( destType->type()==NamedType::Type::SignedInteger ) {
        if( sourceType->size()>=destType->size() ) {
            if( implicitOnly )
                REPORT_ERROR();

            codeGen->truncateInteger(
                    castResult, sourceExpression, StaticType::allocate(sourceType->id()),
                    StaticType::allocate(destType->id()) );
        } else {
            codeGen->expandIntegerUnsigned(
                    castResult, sourceExpression, StaticType::allocate(sourceType->id()),
                    StaticType::allocate(destType->id()) );
        }
    } else if( destType->type()==NamedType::Type::UnsignedInteger ) {
        if( sourceType->size()>destType->size() ) {
            if( implicitOnly )
                REPORT_ERROR();

            codeGen->truncateInteger(
                    castResult, sourceExpression, StaticType::allocate(sourceType->id()),
                    StaticType::allocate(destType->id()) );
        } else {
            codeGen->expandIntegerUnsigned(
                    castResult, sourceExpression, StaticType::allocate(sourceType->id()),
                    StaticType::allocate(destType->id()) );
        }
    } else {
        ABORT() << "TODO implement";
    }

    return castResult;
#undef REPORT_ERROR
}

ExpressionId codeGenCast(
        FunctionGen *codeGen, ExpressionId sourceExpression, StaticType::Ptr sourceType, ExpectedType destType,
        const Tokenizer::Token &expressionSource, bool implicitOnly )
{
    return codeGenCast( codeGen, sourceExpression, sourceType, destType.type, expressionSource, implicitOnly );
}

ExpressionId codeGenCast(
        FunctionGen *codeGen, ExpressionId sourceExpression, StaticType::Ptr sourceType, StaticType::Ptr destType,
        const Tokenizer::Token &expressionSource, bool implicitOnly )
{
    // Fastpath
    if( sourceType==destType )
        return sourceExpression;

    const NamedType *namedSource = LookupContext::lookupType( sourceType->getId() );
    const NamedType *namedDest = LookupContext::lookupType( destType->getId() );

    ASSERT( namedSource!=nullptr );
    ASSERT( namedDest!=nullptr );

    if( namedSource->type()==NamedType::Type::SignedInteger )
        return codeGenCast_SignedIntSource(codeGen, sourceExpression, namedSource, namedDest, expressionSource, implicitOnly);

    if( namedSource->type()==NamedType::Type::UnsignedInteger ) {
        return codeGenCast_UnsignedIntSource(codeGen, sourceExpression, namedSource, namedDest, expressionSource, implicitOnly);
    }

    ABORT() << "Unreachable code reached";
}
