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
#include "ast/signed_int_value_range.h"
#include "ast/unsigned_int_value_range.h"

#include <practical-errors.h>

using namespace PracticalSemanticAnalyzer;

namespace AST::ExpressionImpl {

Literal::Literal( const NonTerminals::Literal &parserLiteral ) :
    parserLiteral( parserLiteral )
{
}

void Literal::buildASTImpl(
        LookupContext &lookupContext, ExpectedResult expectedResult, unsigned &weight, unsigned weightLimit )
{
    switch( parserLiteral.token.token ) {
    case Tokenizer::Tokens::LITERAL_INT_10:
        {
            auto ptr = safenew< LiteralInt >();
            LiteralInt *literalInt = ptr.get();
            impl = std::move( ptr );
            literalInt->parseInt10( this, weight, weightLimit, expectedResult );
        }
        break;
    case Tokenizer::Tokens::LITERAL_INT_16:
    case Tokenizer::Tokens::LITERAL_INT_2:
        ABORT()<<"TODO implement";
        break;
    case Tokenizer::Tokens::RESERVED_TRUE:
    case Tokenizer::Tokens::RESERVED_FALSE:
        {
            auto ptr = safenew< LiteralBool >();
            LiteralBool *literalBool = ptr.get();
            impl = std::move( ptr );
            literalBool->parseBool( this, weight, weightLimit, expectedResult );
        }
        break;
    default:
        ABORT()<<"Non literal parsed as literal";
    }
}

ExpressionId Literal::codeGenImpl( PracticalSemanticAnalyzer::FunctionGen *functionGen ) {
    ASSERT( impl )<<"Literal::codeGen called without calling buildAST first";
    return impl->codeGen( this, functionGen );
}

// Private
ExpressionId Literal::LiteralInt::codeGen( Literal *owner, PracticalSemanticAnalyzer::FunctionGen *functionGen ) {
    ExpressionId id = allocateId();

    functionGen->setLiteral( id, result, owner->metadata.type );

    return id;
}

void Literal::LiteralInt::parseInt10(
        Literal *owner, unsigned &weight, unsigned weightLimit, ExpectedResult expectedResult )
{
    result = 0;

    for( char c: owner->parserLiteral.token.text ) {
        if( c=='_' )
            continue;

        if( result > std::numeric_limits<LongEnoughInt>::max()/10 )
            throw IllegalLiteral(
                    "Literal integer too big",
                    owner->parserLiteral.token.line,
                    owner->parserLiteral.token.col );

        ASSERT( c>='0' && c<='9' ) << "Decimal literal has character '"<<c<<"' out of allowed range";
        result *= 10;
        result += c-'0';
    }

    parseInt( owner, weight, weightLimit, expectedResult );
}

void Literal::LiteralInt::parseInt(
        Literal *owner, unsigned &weight, unsigned weightLimit, ExpectedResult expectedResult )
{
    ASSERT( !owner->metadata.type )<<"Cannot reuse AST nodes";

    StaticTypeImpl::CPtr naturalType;
    if( result>std::numeric_limits<uint32_t>::max() ) {
        ASSERT( result<=std::numeric_limits<uint64_t>::max() );
        naturalType = AST::getBuiltinCtx().lookupType("U64");
    } else if( result>std::numeric_limits<uint16_t>::max() ) {
        naturalType = AST::getBuiltinCtx().lookupType("U32");
    } else if( result>std::numeric_limits<uint8_t>::max() ) {
        naturalType = AST::getBuiltinCtx().lookupType("U16");
    } else {
        naturalType = AST::getBuiltinCtx().lookupType("U8");
    }
    ASSERT( naturalType );

    owner->metadata.valueRange = UnsignedIntValueRange::allocate( result, result );

    if( !expectedResult ) {
        static const StaticTypeImpl::CPtr DefaultLiteralIntType =
                AST::getBuiltinCtx().lookupType("U64");
        static const unsigned DefaultLiteralIntWeight =
                std::get< const StaticType::Scalar *>( DefaultLiteralIntType->getType() )->getLiteralWeight();

        // If nothing is expected, return the maximal unsigned type
        owner->metadata.type =  DefaultLiteralIntType;
        ASSERT( owner->metadata.type );

        weight += DefaultLiteralIntWeight;
        return;
    }

    auto expectedType = expectedResult.getType()->getType();
    auto expectedScalar = std::get_if< const StaticType::Scalar * >(&expectedType);
    if( expectedScalar ) {
        if( (*expectedScalar)->getType() == StaticType::Scalar::Type::SignedInt ) {
            LongEnoughInt limit=1;
            limit <<= (*expectedScalar)->getSize()-1;
            if( result<limit ) {
                owner->metadata.type = expectedResult.getType();
                owner->metadata.valueRange = SignedIntValueRange::allocate( result, result );

                weight += (*expectedScalar)->getLiteralWeight();

                return;
            }
        } else if( (*expectedScalar)->getType() == StaticType::Scalar::Type::UnsignedInt ) {
            ASSERT( (*expectedScalar)->getSize()<=64 );

            weight += (*expectedScalar)->getLiteralWeight();

            if( (*expectedScalar)->getSize()==64 ) {
                owner->metadata.type = expectedResult.getType();
                return;
            }

            LongEnoughInt limit=1;
            limit <<= (*expectedScalar)->getSize();

            if( result < limit ) {
                owner->metadata.type = expectedResult.getType();
                return;
            }
        }
    } else {
        // A type is expected, but it is not a scalar. Put in the default type and let the caller try a cast
        owner->metadata.type = naturalType;
        ASSERT( owner->metadata.type );

        weight += std::get< const StaticType::Scalar *>( naturalType->getType() )->getLiteralWeight();
    }
}

ExpressionId Literal::LiteralBool::codeGen( Literal *owner, PracticalSemanticAnalyzer::FunctionGen *functionGen ) {
    ExpressionId id = allocateId();
    functionGen->setLiteral( id, result );

    return id;
}

void Literal::LiteralBool::parseBool(
        Literal *owner, unsigned &weight, unsigned weightLimit, ExpectedResult expectedResult )
{
    switch( owner->parserLiteral.token.token ) {
    case Tokenizer::Tokens::RESERVED_TRUE:
        result = true;
        break;
    case Tokenizer::Tokens::RESERVED_FALSE:
        result = false;
        break;
    default:
        ABORT()<<"Unreachable code reached";
    }

    owner->metadata.type = AST::getBuiltinCtx().lookupType("Bool");
    owner->metadata.valueRange = BoolValueRange::allocate( result==false, result==true );
}

} // namespace AST::ExpressionImpl
