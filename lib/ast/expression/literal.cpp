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
    return parserLiteral.token.location;
}

void Literal::buildASTImpl(
        LookupContext &lookupContext, ExpectedResult expectedResult, Weight &weight, Weight weightLimit )
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
    case Tokenizer::Tokens::LITERAL_STRING:
        {
            auto ptr = safenew< LiteralString >();
            LiteralString *literalString = ptr.get();
            impl = std::move( ptr );
            literalString->parse( this, weight, weightLimit, expectedResult );
        }
        break;
    case Tokenizer::Tokens::RESERVED_NULL:
        {
            auto ptr = safenew< LiteralNull >();
            LiteralNull *literalNull = ptr.get();
            impl = std::move( ptr );
            literalNull->buildAst( this, weight, weightLimit, expectedResult );
        }
        break;
    default:
        ABORT()<<"Non literal parsed as literal: "<<parserLiteral.token.token;
    }
}

ExpressionId Literal::codeGenImpl( PracticalSemanticAnalyzer::FunctionGen *functionGen ) const {
    ASSERT( impl )<<"Literal::codeGen called without calling buildAST first";
    return impl->codeGen( this, functionGen );
}

// Private
ExpressionId Literal::LiteralInt::codeGen(
        const Literal *owner, PracticalSemanticAnalyzer::FunctionGen *functionGen ) const
{
    ExpressionId id = allocateId();

    functionGen->setLiteral( id, result, owner->metadata.type );

    return id;
}

void Literal::LiteralInt::parseInt10(
        Literal *owner, Weight &weight, Weight weightLimit, ExpectedResult expectedResult )
{
    result = 0;

    for( char c: owner->parserLiteral.token.text ) {
        static constexpr LongEnoughInt
                LimitDivided = std::numeric_limits<LongEnoughInt>::max() / 10,
                LimitTruncated = LimitDivided * 10,
                LimitLastDigit = std::numeric_limits<LongEnoughInt>::max() % 10;

        if( c=='_' )
            continue;

        if( result > LimitDivided )
            throw IllegalLiteral(
                    "Literal integer too big",
                    owner->parserLiteral.token.location );

        ASSERT( c>='0' && c<='9' ) << "Decimal literal has character '"<<c<<"' out of allowed range";
        result *= 10;
        if( result == LimitTruncated && c-'0'>LimitLastDigit )
            throw IllegalLiteral(
                    "Literal integer too big",
                    owner->parserLiteral.token.location );

        result += c-'0';
    }

    parseInt( owner, weight, weightLimit, expectedResult );
}

void Literal::LiteralInt::parseInt(
        Literal *owner, Weight &weight, Weight weightLimit, ExpectedResult expectedResult )
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
        static const Weight DefaultLiteralIntWeight =
                Weight( std::get< const StaticType::Scalar *>( DefaultLiteralIntType->getType() )->getLiteralWeight(), 0 );

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

                weight += Weight( (*expectedScalar)->getLiteralWeight(), 0 );

                return;
            }
        } else if( (*expectedScalar)->getType() == StaticType::Scalar::Type::UnsignedInt ) {
            ASSERT( (*expectedScalar)->getSize()<=64 );

            if( (*expectedScalar)->getSize()==64 ) {
                weight += Weight( (*expectedScalar)->getLiteralWeight(), 0 );
                owner->metadata.type = expectedResult.getType();
                return;
            }

            LongEnoughInt limit=1;
            limit <<= (*expectedScalar)->getSize();

            if( result < limit ) {
                weight += Weight( (*expectedScalar)->getLiteralWeight(), 0 );
                owner->metadata.type = expectedResult.getType();
                return;
            }
        }
    }

    ASSERT( ! owner->metadata.type );

    // A type is expected, but it is not a scalar (or otherwise doesn't work). Put in the default type and
    // let the caller try a cast
    owner->metadata.type = naturalType;
    ASSERT( owner->metadata.type );

    weight += Weight( std::get< const StaticType::Scalar *>( naturalType->getType() )->getLiteralWeight() );
}

ExpressionId Literal::LiteralBool::codeGen(
        const Literal *owner, PracticalSemanticAnalyzer::FunctionGen *functionGen ) const
{
    ExpressionId id = allocateId();
    functionGen->setLiteral( id, result );

    return id;
}

void Literal::LiteralBool::parseBool(
        Literal *owner, Weight &weight, Weight weightLimit, ExpectedResult expectedResult )
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

ExpressionId Literal::LiteralString::codeGen(
        const Literal *owner, PracticalSemanticAnalyzer::FunctionGen *functionGen ) const
{
    ExpressionId id = allocateId();
    functionGen->setLiteral( id, result );

    return id;
}

void Literal::LiteralString::parse(
        Literal *owner, Weight &weight, Weight weightLimit, ExpectedResult expectedResult )
{
    static const std::unordered_map<
            State,
            State (Literal::LiteralString::*)( String, int &, const SourceLocation & )
        > stateParsers
    {
        { State::None, &LiteralString::parserNone },
        { State::Backslash, &LiteralString::parserBackslash }
    };

    ASSERT( owner->parserLiteral.token.token == Tokenizer::Tokens::LITERAL_STRING );
    ASSERT( result.empty() );

    String body = owner->parserLiteral.token.text;
    ASSERT( body.size()>2 );
    ASSERT( body[0]=='"' );
    ASSERT( body[body.size()-1]=='"' );

    SourceLocation location = owner->parserLiteral.token.location;

    // Strip leading and trailing quotes
    body = body.subslice( 1, body.size()-1 );
    ++location;

    ASSERT( state == State::None );

    int stateData = 0;
    while( body.size()>0 ) {
        state = (this ->* stateParsers.at(state))( body, stateData, location );
        body = body.subslice(1);
        ++location;
    }

    result += '\0';

    auto c8Type = AST::getBuiltinCtx().lookupType("C8");
    owner->metadata.type = StaticTypeImpl::allocate( PointerTypeImpl( c8Type ) );
    owner->metadata.valueRange = new PointerValueRange( c8Type->defaultRange() );
}

Literal::LiteralString::State Literal::LiteralString::parserNone(
        String source, int &stateData, const SourceLocation &location )
{
    switch( source[0] ) {
    case '\\':
        return State::Backslash;
    }

    result += source[0];
    return State::None;
}

Literal::LiteralString::State Literal::LiteralString::parserBackslash(
        String source, int &stateData, const SourceLocation &location )
{
    switch( source[0] ) {
    case '\'':
    case '"':
    case '?':
    case '\\':
        result += source[0];
        return State::None;
    case '0':
        result += char(0); // NUL (Null)
        return State::None;
    case 'a':
        result += 7; // BEL (Bell)
        return State::None;
    case 'b':
        result += 8; // BS (Backspace)
        return State::None;
    case 't':
        result += 9; // HT (Horizontal tab)
        return State::None;
    case 'n':
        result += 10; // NL (Newline)
        return State::None;
    case 'v':
        result += 11; // VT (Vertical tab)
        return State::None;
    case 'f':
        result += 12; // FF (Form feed)
        return State::None;
    case 'r':
        result += 13; // CR (Carriage return)
        return State::None;
    case 'x':
        stateData = 0;
        result += char(0);
        return State::Hex;
    }

    throw InvalidEscapeSequence(location);
}

ExpressionId Literal::LiteralNull::codeGen(
        const Literal *owner, PracticalSemanticAnalyzer::FunctionGen *functionGen ) const
{
    ExpressionId id = allocateId();

    functionGen->setLiteralNull( id, owner->metadata.type );
    return id;
}

void Literal::LiteralNull::buildAst(
        Literal *owner, Weight &weight, Weight weightLimit, ExpectedResult expectedResult )
{
    if( ! expectedResult )
        throw PointerExpected( nullptr, owner->parserLiteral.token.location );

    auto expectedType = expectedResult.getType();
    auto expectedTypeType = expectedType->getType();
    auto pointedType = std::get_if<const StaticType::Pointer *>(&expectedTypeType);
    if( pointedType == nullptr )
        throw PointerExpected( expectedType, owner->parserLiteral.token.location );

    owner->metadata.type = expectedType;
    owner->metadata.valueRange = new PointerValueRange( nullptr );
}

} // namespace AST::ExpressionImpl
