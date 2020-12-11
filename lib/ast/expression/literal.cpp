/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "literal.h"

#include "ast/ast.h"
#include "ast/bool_value_range.h"
#include "ast/pointers.h"
#include "ast/signed_int_value_range.h"
#include "ast/unsigned_int_value_range.h"

#include <practical/errors.h>

using namespace PracticalSemanticAnalyzer;

namespace AST::ExpressionImpl {

Literal::Literal( const NonTerminals::Literal &parserLiteral ) :
    parserLiteral( parserLiteral )
{
}

SourceLocation Literal::getLocation() const {
    return parserLiteral.getLocation();
}

void Literal::buildASTImpl(
        LookupContext &lookupContext, ExpectedResult expectedResult, Weight &weight, Weight weightLimit )
{
    struct Visitor {
        Literal &_this;
        Weight &weight;
        Weight weightLimit;
        const ExpectedResult &expectedResult;

        void operator()( const NonTerminals::LiteralInt &literal ) {
            _this.buildAstInt( literal, weight, weightLimit, expectedResult );
        }

        void operator()( const NonTerminals::LiteralBool &literal ) {
            _this.buildAstBool( literal, weight, weightLimit, expectedResult );
        }

        void operator()( const NonTerminals::LiteralPointer &literal ) {
            _this.buildAstPointer( literal, weight, weightLimit, expectedResult );
        }

        void operator()( const NonTerminals::LiteralString &literal ) {
            _this.buildAstString( literal, weight, weightLimit, expectedResult );
        }
    };

    std::visit(
            Visitor{
                ._this = *this, .weight = weight, .weightLimit = weightLimit,
                .expectedResult = expectedResult
            },
            parserLiteral.literal );
}

ExpressionId Literal::codeGenImpl( PracticalSemanticAnalyzer::FunctionGen *functionGen ) const {
    struct Visitor {
        const Literal &_this;
        PracticalSemanticAnalyzer::FunctionGen *functionGen;

        ExpressionId operator()( const NonTerminals::LiteralInt &literal ) {
            return _this.codeGenInt( literal, functionGen );
        }

        ExpressionId operator()( const NonTerminals::LiteralBool &literal ) {
            return _this.codeGenBool( literal, functionGen );
        }

        ExpressionId operator()( const NonTerminals::LiteralPointer &literal ) {
            return _this.codeGenPointer( literal, functionGen );
        }

        ExpressionId operator()( const NonTerminals::LiteralString &literal ) {
            return _this.codeGenString( literal, functionGen );
        }
    };

    return std::visit( Visitor{ ._this = *this, .functionGen = functionGen }, parserLiteral.literal );
}

// Private
void Literal::buildAstInt(
        const NonTerminals::LiteralInt &literal, Weight &weight, Weight weightLimit,
        ExpectedResult expectedResult )
{
    ASSERT( !metadata.type )<<"Cannot reuse AST nodes";

    StaticTypeImpl::CPtr naturalType;
    if( literal.value>std::numeric_limits<uint32_t>::max() ) {
        ASSERT( literal.value<=std::numeric_limits<uint64_t>::max() );
        naturalType = AST::getBuiltinCtx().lookupType("U64");
    } else if( literal.value>std::numeric_limits<uint16_t>::max() ) {
        naturalType = AST::getBuiltinCtx().lookupType("U32");
    } else if( literal.value>std::numeric_limits<uint8_t>::max() ) {
        naturalType = AST::getBuiltinCtx().lookupType("U16");
    } else {
        naturalType = AST::getBuiltinCtx().lookupType("U8");
    }
    ASSERT( naturalType );

    metadata.valueRange = UnsignedIntValueRange::allocate( literal.value, literal.value );

    if( !expectedResult ) {
        static const StaticTypeImpl::CPtr DefaultLiteralIntType =
                AST::getBuiltinCtx().lookupType("U64");
        static const Weight DefaultLiteralIntWeight =
                Weight( std::get< const StaticType::Scalar *>( DefaultLiteralIntType->getType() )->getLiteralWeight(), 0 );

        // If nothing is expected, return the maximal unsigned type
        metadata.type =  DefaultLiteralIntType;
        ASSERT( metadata.type );

        weight += DefaultLiteralIntWeight;
        return;
    }

    auto expectedType = expectedResult.getType()->getType();
    auto expectedScalar = std::get_if< const StaticType::Scalar * >(&expectedType);
    if( expectedScalar ) {
        if( (*expectedScalar)->getType() == StaticType::Scalar::Type::SignedInt ) {
            LongEnoughInt limit=1;
            limit <<= (*expectedScalar)->getSize()-1;
            if( literal.value<limit ) {
                metadata.type = expectedResult.getType();
                metadata.valueRange = SignedIntValueRange::allocate( literal.value, literal.value );

                weight += Weight( (*expectedScalar)->getLiteralWeight(), 0 );

                return;
            }
        } else if( (*expectedScalar)->getType() == StaticType::Scalar::Type::UnsignedInt ) {
            ASSERT( (*expectedScalar)->getSize()<=64 );

            if( (*expectedScalar)->getSize()==64 ) {
                weight += Weight( (*expectedScalar)->getLiteralWeight(), 0 );
                metadata.type = expectedResult.getType();
                return;
            }

            LongEnoughInt limit=1;
            limit <<= (*expectedScalar)->getSize();

            if( literal.value < limit ) {
                weight += Weight( (*expectedScalar)->getLiteralWeight(), 0 );
                metadata.type = expectedResult.getType();
                return;
            }
        }
    }

    ASSERT( ! metadata.type );

    // A type is expected, but it is not a scalar (or otherwise doesn't work). Put in the default type and
    // let the caller try a cast
    metadata.type = naturalType;
    ASSERT( metadata.type );

    weight += Weight( std::get< const StaticType::Scalar *>( naturalType->getType() )->getLiteralWeight() );
}

void Literal::buildAstBool(
        const NonTerminals::LiteralBool &literal, Weight &weight, Weight weightLimit,
        ExpectedResult expectedResult )
{
    metadata.type = AST::getBuiltinCtx().lookupType("Bool");
    metadata.valueRange = BoolValueRange::allocate( literal.value==false, literal.value==true );
}

void Literal::buildAstPointer(
        const NonTerminals::LiteralPointer &literal, Weight &weight, Weight weightLimit,
        ExpectedResult expectedResult )
{
    if( ! expectedResult )
        throw PointerExpected( nullptr, literal.token->location );

    auto expectedType = expectedResult.getType();
    auto expectedTypeType = expectedType->getType();
    auto pointedType = std::get_if<const StaticType::Pointer *>(&expectedTypeType);
    if( pointedType == nullptr )
        throw PointerExpected( expectedType, literal.token->location );

    metadata.type = expectedType;
    metadata.valueRange = new PointerValueRange( nullptr );
}

void Literal::buildAstString(
        const NonTerminals::LiteralString &literal, Weight &weight, Weight weightLimit,
        ExpectedResult expectedResult )
{
    auto c8Type = AST::getBuiltinCtx().lookupType("C8");
    metadata.type = StaticTypeImpl::allocate( PointerTypeImpl( c8Type ) );
    metadata.valueRange = new PointerValueRange( c8Type->defaultRange() );
}

ExpressionId Literal::codeGenInt(
        const NonTerminals::LiteralInt &literal, PracticalSemanticAnalyzer::FunctionGen *functionGen ) const
{
    ExpressionId id = allocateId();

    functionGen->setLiteral( id, literal.value, metadata.type );

    return id;
}

ExpressionId Literal::codeGenBool(
        const NonTerminals::LiteralBool &literal,
        PracticalSemanticAnalyzer::FunctionGen *functionGen ) const
{
    ExpressionId id = allocateId();
    functionGen->setLiteral( id, literal.value );

    return id;
}

ExpressionId Literal::codeGenString(
        const NonTerminals::LiteralString &literal,
        PracticalSemanticAnalyzer::FunctionGen *functionGen ) const
{
    ExpressionId id = allocateId();
    functionGen->setLiteral( id, literal.value + '\0' );

    return id;
}

ExpressionId Literal::codeGenPointer(
        const NonTerminals::LiteralPointer &literal,
        PracticalSemanticAnalyzer::FunctionGen *functionGen ) const
{
    ExpressionId id = allocateId();

    functionGen->setLiteralNull( id, metadata.type );
    return id;
}

} // namespace AST::ExpressionImpl
