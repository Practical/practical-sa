/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/expression/binary_op.h"

#include "ast/operators/plus.h"
#include "tokenizer.h"

#include <experimental/array>

namespace AST::ExpressionImpl {

// Non member private helpers
static std::unordered_map< Tokenizer::Tokens, std::string > operatorNames;

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

String opToFuncName( Tokenizer::Tokens token ) {
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
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_ASTERISK, "__opMultiply" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_BIT_AND, "__opBitAnd" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_BIT_OR, "__opBitOr" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_BIT_XOR, "__opBitXor" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_DIVIDE, "__opDiv" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_EQUALS, "__opEquals" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_GREATER_THAN, "__opGT" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_GREATER_THAN_EQ, "__opGE" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_LESS_THAN, "__opLT" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_LESS_THAN_EQ, "__opLE" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_LOGIC_AND, "__opAnd" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_LOGIC_OR, "__opOr" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_MINUS, "__opMinus" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_MODULOUS, "__opMod" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_NOT_EQUALS, "__opEQ" );

    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_PLUS, "__opPlus" );
    defineMatchingPairs( Operators::bPlusCodegen, Operators::bPlusUnsignedVrp, inserter.first->second, unsignedTypes, builtinCtx );
    defineMatchingPairs( Operators::bPlusCodegen, Operators::bPlusSignedVrp, inserter.first->second, signedTypes, builtinCtx );

    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_SHIFT_LEFT, "__opShiftLeft" );
    inserter = operatorNames.emplace( Tokenizer::Tokens::OP_SHIFT_RIGHT, "__opShiftRight" );
}

BinaryOp::BinaryOp( const NonTerminals::Expression::BinaryOperator &parserOp ) :
    parserOp(parserOp)
{}

size_t BinaryOp::getLine() const {
    return parserOp.op->line;
}

size_t BinaryOp::getCol() const {
    return parserOp.op->col;
}

// Protected methods
void BinaryOp::buildASTImpl(
        LookupContext &lookupContext, ExpectedResult expectedResult, unsigned &weight, unsigned weightLimit )
{
    String baseName = opToFuncName( parserOp.op->token );
    auto identifier = lookupContext.lookupIdentifier( baseName );
    ASSERT( identifier );
    const LookupContext::Function &function =
            std::get<LookupContext::Function>(*identifier);

    resolver.resolveOverloads( lookupContext, expectedResult, function.overloads, weight, weightLimit, metadata,
            { parserOp.operands[0].get(), parserOp.operands[1].get() }, parserOp.op );
}

ExpressionId BinaryOp::codeGenImpl( PracticalSemanticAnalyzer::FunctionGen *functionGen ) const {
    return resolver.codeGen( functionGen );
}

} // namespace AST::ExpressionImpl
