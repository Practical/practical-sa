#include "casts.h"

#include "asserts.h"
#include "ast.h"
#include "dummy_codegen_impl.h"
#include "lookup_context.h"
#include "practical-errors.h"

using namespace PracticalSemanticAnalyzer;

void checkImplicitCastAllowed(
        ExpressionId id, const StaticType &sourceType, const StaticType &destType, const Tokenizer::Token &expressionSource)
{
    codeGenCast( &dummyFunctionGen, id, sourceType, destType, expressionSource, true );
}

static ExpressionId codeGenCast_SignedIntSource(
        FunctionGen *codeGen, ExpressionId sourceExpression, const NamedType *sourceType, const NamedType *destType,
        const Tokenizer::Token &expressionSource, bool implicitOnly )
{
#define REPORT_ERROR() throw CastNotAllowed(StaticType(sourceType->id()), StaticType(destType->id()), implicitOnly, expressionSource.line, expressionSource.col)
    if( destType->type()!=NamedType::Type::SignedInteger )
        REPORT_ERROR();

    ExpressionId castResult = expressionIdAllocator.allocate();
    if( sourceType->size()>destType->size() ) {
        if( implicitOnly )
            REPORT_ERROR();

        codeGen->truncateInteger( castResult, sourceExpression, StaticType(sourceType->id()), StaticType(destType->id()) );
    } else {
        codeGen->expandIntegerSigned( castResult, sourceExpression, StaticType(sourceType->id()), StaticType(destType->id()) );
    }

    return castResult;
#undef REPORT_ERROR
}

static ExpressionId codeGenCast_UnsignedIntSource(
        FunctionGen *codeGen, ExpressionId sourceExpression, const NamedType *sourceType, const NamedType *destType,
        const Tokenizer::Token &expressionSource, bool implicitOnly )
{
#define REPORT_ERROR() throw CastNotAllowed(StaticType(sourceType->id()), StaticType(destType->id()), implicitOnly, expressionSource.line, expressionSource.col)
    ExpressionId castResult = expressionIdAllocator.allocate();

    if( destType->type()==NamedType::Type::SignedInteger ) {
        if( sourceType->size()>=destType->size() ) {
            if( implicitOnly )
                REPORT_ERROR();

            codeGen->truncateInteger( castResult, sourceExpression, StaticType(sourceType->id()), StaticType(destType->id()) );
        } else {
            codeGen->expandIntegerUnsigned(
                    castResult, sourceExpression, StaticType(sourceType->id()), StaticType(destType->id()) );
        }
    } else if( destType->type()==NamedType::Type::UnsignedInteger ) {
        if( sourceType->size()>destType->size() ) {
            if( implicitOnly )
                REPORT_ERROR();

            codeGen->truncateInteger( castResult, sourceExpression, StaticType(sourceType->id()), StaticType(destType->id()) );
        } else {
            codeGen->expandIntegerUnsigned(
                    castResult, sourceExpression, StaticType(sourceType->id()), StaticType(destType->id()) );
        }
    } else {
        ABORT() << "TODO implement";
    }

    return castResult;
#undef REPORT_ERROR
}

ExpressionId codeGenCast(
        FunctionGen *codeGen, ExpressionId sourceExpression, const StaticType &sourceType, const StaticType &destType,
        const Tokenizer::Token &expressionSource, bool implicitOnly )
{
    // Fastpath
    if( sourceType==destType )
        return sourceExpression;

    const NamedType *namedSource = LookupContext::lookupType( sourceType.getId() );
    const NamedType *namedDest = LookupContext::lookupType( destType.getId() );

    ASSERT( namedSource!=nullptr );
    ASSERT( namedDest!=nullptr );

    if( namedSource->type()==NamedType::Type::SignedInteger )
        return codeGenCast_SignedIntSource(codeGen, sourceExpression, namedSource, namedDest, expressionSource, implicitOnly);

    if( namedSource->type()==NamedType::Type::UnsignedInteger ) {
        return codeGenCast_UnsignedIntSource(codeGen, sourceExpression, namedSource, namedDest, expressionSource, implicitOnly);
    }

    ABORT() << "Unreachable code reached";
}
