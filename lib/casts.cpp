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

// Forward declaration
static Expression codeGenCast(
        FunctionGen *codeGen, const Expression &sourceExpression, const LookupContext::NamedType *namedSource,
        const LookupContext::NamedType *namedDest, StaticType::Ptr destType, const Tokenizer::Token &expressionSource,
        bool implicitOnly );

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

static Expression codeGenCast_ValueRange(
        FunctionGen *codeGen, const Expression &sourceExpression, const LookupContext::NamedType *sourceType,
        const LookupContext::NamedType *destNamedType, StaticType::Ptr destStaticType,
        const Tokenizer::Token &expressionSource )
{
#define REPORT_ERROR() throw CastNotAllowed(sourceExpression.type, destStaticType, true, expressionSource.line, expressionSource.col)
    ASSERT( sourceExpression.valueRange )<<"Called value range based cast on expression with no value range";
    ASSERT( destNamedType->range() )<<
            "Asked to convert to "<<destNamedType->name()<<
            " based on value range, but type doesn't have range information";
    if( not sourceExpression.valueRange->containedIn(* destNamedType->range()) )
    {
        REPORT_ERROR();
    }

    // The range matches. Use explicit cast to perform the actual conversion
    return codeGenCast( codeGen, sourceExpression, sourceType, destNamedType, destStaticType, expressionSource, false); 
#undef REPORT_ERROR
}

static Expression codeGenCast_SignedIntSource(
        FunctionGen *codeGen, const Expression &sourceExpression, const LookupContext::NamedType *sourceType,
        const LookupContext::NamedType *destNamedType, StaticType::Ptr destStaticType,
        const Tokenizer::Token &expressionSource, bool implicitOnly )
{
#define REPORT_ERROR() throw CastNotAllowed(sourceExpression.type, destStaticType, implicitOnly, expressionSource.line, expressionSource.col)
    if( destNamedType->type()!=NamedType::Type::SignedInteger )
        REPORT_ERROR();

    Expression castResult{ StaticType::Ptr(destStaticType) };
    if( sourceType->size()>destNamedType->size() ) {
        if( implicitOnly )
            REPORT_ERROR();

        codeGen->truncateInteger( castResult.id, sourceExpression.id, sourceExpression.type, destStaticType );

        ASSERT( ! castResult.valueRange )<<"Cast result value range not known but set to "<<*castResult.valueRange;
    } else {
        codeGen->expandIntegerSigned( castResult.id, sourceExpression.id, sourceExpression.type, destStaticType );

        if( sourceExpression.valueRange )
            castResult.valueRange = sourceExpression.valueRange;
        else
            castResult.valueRange = sourceType->range();
    }

    return castResult;
#undef REPORT_ERROR
}

static Expression codeGenCast_UnsignedIntSource(
        FunctionGen *codeGen, const Expression &sourceExpression, const LookupContext::NamedType *sourceType,
        const LookupContext::NamedType *destNamedType, StaticType::Ptr destStaticType,
        const Tokenizer::Token &expressionSource, bool implicitOnly )
{
#define REPORT_ERROR() throw CastNotAllowed(sourceExpression.type, destStaticType, implicitOnly, expressionSource.line, expressionSource.col)
    Expression castResult{ StaticType::Ptr(destStaticType) };

    if( destNamedType->type()==NamedType::Type::SignedInteger ) {
        if( sourceType->size()>=destNamedType->size() ) {
            if( implicitOnly )
                REPORT_ERROR();

            codeGen->truncateInteger( castResult.id, sourceExpression.id, sourceExpression.type, destStaticType );

            ASSERT( ! castResult.valueRange )<<"Cast result value range not known but set to "<<*castResult.valueRange;
        } else {
            codeGen->expandIntegerUnsigned( castResult.id, sourceExpression.id, sourceExpression.type, destStaticType );

            if( sourceExpression.valueRange )
                castResult.valueRange = sourceExpression.valueRange;
            else
                castResult.valueRange = sourceType->range();
        }
    } else if( destNamedType->type()==NamedType::Type::UnsignedInteger ) {
        if( sourceType->size()>destNamedType->size() ) {
            if( implicitOnly )
                REPORT_ERROR();

            codeGen->truncateInteger( castResult.id, sourceExpression.id, sourceExpression.type, destStaticType );

            ASSERT( ! castResult.valueRange )<<"Cast result value range not known but set to "<<*castResult.valueRange;
        } else {
            codeGen->expandIntegerUnsigned( castResult.id, sourceExpression.id, sourceExpression.type, destStaticType );

            if( sourceExpression.valueRange )
                castResult.valueRange = sourceExpression.valueRange;
            else
                castResult.valueRange = sourceType->range();
        }
    } else {
        ABORT() << "TODO implement";
    }

    return castResult;
#undef REPORT_ERROR
}

static Expression codeGenCast(
        FunctionGen *codeGen, const Expression &sourceExpression, const LookupContext::NamedType *namedSource,
        const LookupContext::NamedType *namedDest, StaticType::Ptr destType, const Tokenizer::Token &expressionSource,
        bool implicitOnly )
{
    if( implicitOnly && sourceExpression.valueRange ) {
        return codeGenCast_ValueRange( codeGen, sourceExpression, namedSource, namedDest, destType, expressionSource );
    }

    if( namedSource->type()==NamedType::Type::SignedInteger )
        return codeGenCast_SignedIntSource(
                codeGen, sourceExpression, namedSource, namedDest, destType, expressionSource, implicitOnly);

    if( namedSource->type()==NamedType::Type::UnsignedInteger ) {
        return codeGenCast_UnsignedIntSource(
                codeGen, sourceExpression, namedSource, namedDest, destType, expressionSource, implicitOnly);
    }

    ABORT() << "Unreachable code reached";
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

    const LookupContext::NamedType *namedSource = LookupContext::lookupType( sourceExpression.type->getId() );
    const LookupContext::NamedType *namedDest = LookupContext::lookupType( destType->getId() );

    ASSERT( namedSource!=nullptr );
    ASSERT( namedDest!=nullptr );

    return codeGenCast( codeGen, sourceExpression, namedSource, namedDest, destType, expressionSource, implicitOnly );
}
