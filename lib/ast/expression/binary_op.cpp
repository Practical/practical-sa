/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/expression/binary_op.h"

#include "ast/operators/algebraic_int.h"
#include "ast/operators/boolean.h"
#include "tokenizer.h"

#include <experimental/array>

namespace AST::ExpressionImpl {

// Non member private helpers
static std::unordered_map< Tokenizer::Tokens, std::string > operatorNames;

static void defineMatchingPairs(
        LookupContext::Function::Definition::CodeGenProto *codeGenerator,
        LookupContext::Function::Definition::VrpProto *calcVrp,
        const std::string &name,
        StaticTypeImpl::CPtr retType,
        Slice<const StaticTypeImpl::CPtr> types,
        LookupContext &builtinCtx)
{
    for( auto &type : types ) {
        builtinCtx.addBuiltinFunction(
                name,
                retType, { type, type },
                codeGenerator, calcVrp );
    }
}

static void defineMatchingPairs(
        LookupContext::Function::Definition::CodeGenProto *codeGenerator,
        LookupContext::Function::Definition::VrpProto *calcVrp,
        const std::string &name,
        Slice<const StaticTypeImpl::CPtr> types,
        LookupContext &builtinCtx)
{
    for( auto &type : types ) {
        builtinCtx.addBuiltinFunction(
                name,
                type, { type, type },
                codeGenerator, calcVrp );
    }
}

static String opToFuncName( Tokenizer::Tokens token ) {
    return operatorNames.at(token);
}

// Static methods
void BinaryOp::init(LookupContext &builtinCtx) {
    auto unsignedTypes = std::experimental::make_array<const StaticTypeImpl::CPtr>(
        builtinCtx.lookupType( "U8" ),
        builtinCtx.lookupType( "U16" ),
        builtinCtx.lookupType( "U32" ),
        builtinCtx.lookupType( "U64" )
    );
    auto signedTypes = std::experimental::make_array<const StaticTypeImpl::CPtr>(
        builtinCtx.lookupType( "S8" ),
        builtinCtx.lookupType( "S16" ),
        builtinCtx.lookupType( "S32" ),
        builtinCtx.lookupType( "S64" )
    );
    const StaticTypeImpl::CPtr boolType = builtinCtx.lookupType( "Bool" );

    auto inserter = operatorNames.emplace( Tokenizer::Tokens::OP_ARROW, "__opArrow" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_ASSIGN, "__opAssign" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_ASSIGN_BIT_AND, "__opAssignBitAnd" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_ASSIGN_BIT_OR, "__opAssignBitOr" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_ASSIGN_BIT_XOR, "__opAssignBitXor" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_ASSIGN_DIVIDE, "__opAssignDivide" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_ASSIGN_LEFT_SHIFT, "__opAssignShiftLeft" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_ASSIGN_MINUS, "__opAssignMinus" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_ASSIGN_MODULOUS, "__opAssignMod" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_ASSIGN_MULTIPLY, "__opAssignMultiply" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_ASSIGN_PLUS, "__opAssignPlus" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_ASSIGN_RIGHT_SHIFT, "__opAssignShiftRight" );

    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_MULTIPLY, "__opMultiply" );
    defineMatchingPairs( Operators::bMultiplyCodegenUnsigned, Operators::bMultiplyUnsignedVrp, inserter.first->second, unsignedTypes, builtinCtx );
    defineMatchingPairs( Operators::bMultiplyCodegenSigned, Operators::bMultiplySignedVrp, inserter.first->second, signedTypes, builtinCtx );

    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_BIT_AND, "__opBitAnd" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_BIT_OR, "__opBitOr" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_BIT_XOR, "__opBitXor" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_DIVIDE, "__opDiv" );
    defineMatchingPairs( Operators::bDivideCodegenUnsigned, Operators::bDivideUnsignedVrp, inserter.first->second, unsignedTypes, builtinCtx );

    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_EQUALS, "__opEquals" );
    defineMatchingPairs( Operators::equalsCodegenInt, Operators::equalsVrpUnsigned, inserter.first->second, boolType, unsignedTypes, builtinCtx );
    defineMatchingPairs( Operators::equalsCodegenInt, Operators::equalsVrpSigned, inserter.first->second, boolType, signedTypes, builtinCtx );

    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_GREATER_THAN, "__opGT" );
    defineMatchingPairs( Operators::greaterThenCodegenUInt, Operators::greaterThenVrpUnsigned, inserter.first->second, boolType, unsignedTypes, builtinCtx );
    defineMatchingPairs( Operators::greaterThenCodegenSInt, Operators::greaterThenVrpSigned, inserter.first->second, boolType, signedTypes, builtinCtx );

    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_GREATER_THAN_EQ, "__opGE" );
    defineMatchingPairs( Operators::greaterThenOrEqualsCodegenUInt, Operators::greaterThenOrEqualsVrpUnsigned, inserter.first->second, boolType, unsignedTypes, builtinCtx );
    defineMatchingPairs( Operators::greaterThenOrEqualsCodegenSInt, Operators::greaterThenOrEqualsVrpSigned, inserter.first->second, boolType, signedTypes, builtinCtx );

    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_LESS_THAN, "__opLT" );
    defineMatchingPairs( Operators::lessThanCodegenUInt, Operators::lessThanVrpUnsigned, inserter.first->second, boolType, unsignedTypes, builtinCtx );
    defineMatchingPairs( Operators::lessThanCodegenSInt, Operators::lessThanVrpSigned, inserter.first->second, boolType, signedTypes, builtinCtx );

    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_LESS_THAN_EQ, "__opLE" );
    defineMatchingPairs( Operators::lessThanOrEqualsCodegenUInt, Operators::lessThanOrEqualsVrpUnsigned, inserter.first->second, boolType, unsignedTypes, builtinCtx );
    defineMatchingPairs( Operators::lessThanOrEqualsCodegenSInt, Operators::lessThanOrEqualsVrpSigned, inserter.first->second, boolType, signedTypes, builtinCtx );

    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_LOGIC_AND, "__opAnd" );
    builtinCtx.addBuiltinFunction(
            inserter.first->second, boolType, { boolType, boolType }, Operators::logicalAnd, Operators::logicalAndVrp );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_LOGIC_OR, "__opOr" );
    builtinCtx.addBuiltinFunction(
            inserter.first->second, boolType, { boolType, boolType }, Operators::logicalOr, Operators::logicalOrVrp );

    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_MINUS, "__opMinus" );
    defineMatchingPairs( Operators::bMinusCodegenUnsigned, Operators::bMinusUnsignedVrp, inserter.first->second, unsignedTypes, builtinCtx );
    defineMatchingPairs( Operators::bMinusCodegenSigned, Operators::bMinusSignedVrp, inserter.first->second, signedTypes, builtinCtx );

    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_MODULOUS, "__opMod" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_NOT_EQUALS, "__opNE" );
    defineMatchingPairs( Operators::notEqualsCodegenInt, Operators::notEqualsVrpUnsigned, inserter.first->second, boolType, unsignedTypes, builtinCtx );
    defineMatchingPairs( Operators::notEqualsCodegenInt, Operators::notEqualsVrpSigned, inserter.first->second, boolType, signedTypes, builtinCtx );

    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_PLUS, "__opPlus" );
    defineMatchingPairs( Operators::bPlusCodegenUnsigned, Operators::bPlusUnsignedVrp, inserter.first->second, unsignedTypes, builtinCtx );
    defineMatchingPairs( Operators::bPlusCodegenSigned, Operators::bPlusSignedVrp, inserter.first->second, signedTypes, builtinCtx );

    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_SHIFT_LEFT, "__opShiftLeft" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_SHIFT_RIGHT, "__opShiftRight" );
}

BinaryOp::BinaryOp( const NonTerminals::Expression::BinaryOperator &parserOp ) :
    parserOp(parserOp)
{}

SourceLocation BinaryOp::getLocation() const {
    return parserOp.op->location;
}

// Protected methods
void BinaryOp::buildASTImpl(
        LookupContext &lookupContext, ExpectedResult expectedResult, Weight &weight, Weight weightLimit )
{
    String baseName = opToFuncName( parserOp.op->token );
    auto identifier = lookupContext.lookupIdentifier( baseName );
    ASSERT( identifier )<<"Binary operator "<<parserOp.op->token<<" is not yet implemented by the compiler";
    const LookupContext::Function &function =
            std::get<LookupContext::Function>(*identifier);

    resolver.resolveOverloads( lookupContext, expectedResult, function.overloads, weight, weightLimit, metadata,
            { parserOp.operands[0].get(), parserOp.operands[1].get() }, parserOp.op );
}

ExpressionId BinaryOp::codeGenImpl( PracticalSemanticAnalyzer::FunctionGen *functionGen ) const {
    return resolver.codeGen( functionGen );
}

} // namespace AST::ExpressionImpl
