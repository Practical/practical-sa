/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * This file is file is copyright (C) 2018-2019 by its authors.
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "codegen_operators.h"

#include "ast.h"
#include "casts.h"
#include "codegen.h"
#include "dummy_codegen_impl.h"
#include "tokenizer.h"

#include "practical-errors.h"

using Tokenizer::Tokens;

namespace CodeGen {

AST::Expression codeGenUnaryPlus(
        LookupContext &ctx, FunctionGen *codeGen, ExpectedType expectedResult,
        const NonTerminals::Expression::UnaryOperator &op)
{
    return codeGenExpression( ctx, codeGen, expectedResult, op.operand.get() );
}

static const std::unordered_map<
    Tokens,
    AST::Expression (*)(
            LookupContext &, FunctionGen *, ExpectedType, const NonTerminals::Expression::UnaryOperator &
    )
> unaryOperatorsMap {
    { Tokens::OP_PLUS, codeGenUnaryPlus },
};

AST::Expression codeGenUnaryOperator(
        LookupContext &ctx, FunctionGen *codeGen, ExpectedType expectedResult,
        const NonTerminals::Expression::UnaryOperator &op)
{
    ASSERT( op.op!=nullptr ) << "Operator codegen called with no operator";

    auto opHandler = unaryOperatorsMap.find( op.op->token );
    ASSERT( opHandler!=unaryOperatorsMap.end() ) <<
            "Code generation for unimplemented unary operator "<<op.op->token<<" at "<<op.op->line<<":"<<op.op->col<<
            "\n";

    return opHandler->second( ctx, codeGen, expectedResult, op );
}

static void binaryOpFindCommonType(
        LookupContext &ctx, FunctionGen *codeGen, ExpectedType expectedResult,
        const NonTerminals::Expression::BinaryOperator &op, AST::Expression &leftOperand, AST::Expression &rightOperand
        )
{
    if( !expectedResult || !expectedResult.mandatory ) {
        // Calculate expression types as they are, and see what comes back

        // TODO: the recursion here has potential to re-calculate the same subtrees many times. Implement cache to speed
        // things up.
        leftOperand = codeGenExpression( ctx, &dummyFunctionGen, expectedResult, op.operand1.get() );
        rightOperand = codeGenExpression( ctx, &dummyFunctionGen, expectedResult, op.operand2.get() );
        expectedResult = findCommonType( op.op, ctx, leftOperand, rightOperand );
    }

    leftOperand = codeGenExpression( ctx, codeGen, expectedResult, op.operand1.get() );
    rightOperand = codeGenExpression( ctx, codeGen, expectedResult, op.operand2.get() );
}

static AST::Expression codeGenBinaryPlus(
        LookupContext &ctx, FunctionGen *codeGen, ExpectedType expectedResult,
        const NonTerminals::Expression::BinaryOperator &op)
{
    AST::Expression leftOperand, rightOperand;
    binaryOpFindCommonType(ctx, codeGen, expectedResult, op, leftOperand, rightOperand);

    ASSERT( leftOperand.type == rightOperand.type );
    AST::Expression result( StaticType::Ptr(leftOperand.type) );
    try {
        auto leftValueRange = leftOperand.getRange(), rightValueRange = rightOperand.getRange();
        if( leftValueRange && rightValueRange )
            result.valueRange = new ValueRange(
                    leftValueRange->minimum + rightValueRange->minimum,
                    leftValueRange->maximum + rightValueRange->maximum );
    } catch( std::overflow_error &err ) {
        // In case of overflow, leave the range blank (i.e. - maximal)
    }
    codeGen->binaryOperatorPlus( result.id, leftOperand.id, rightOperand.id, result.type );

    return result;
}

static AST::Expression codeGenBinaryMinus(
        LookupContext &ctx, FunctionGen *codeGen, ExpectedType expectedResult,
        const NonTerminals::Expression::BinaryOperator &op)
{
    AST::Expression leftOperand, rightOperand;
    binaryOpFindCommonType(ctx, codeGen, expectedResult, op, leftOperand, rightOperand);

    ASSERT( leftOperand.type == rightOperand.type );
    AST::Expression result( StaticType::Ptr(leftOperand.type) );
    try {
        auto leftValueRange = leftOperand.getRange(), rightValueRange = rightOperand.getRange();
        if( leftValueRange && rightValueRange )
            result.valueRange = new ValueRange(
                    leftValueRange->minimum - rightValueRange->maximum,
                    leftValueRange->maximum - rightValueRange->minimum );
    } catch( std::overflow_error &err ) {
        // In case of overflow, leave the range blank (i.e. - maximal)
    }

    codeGen->binaryOperatorMinus( result.id, leftOperand.id, rightOperand.id, result.type );

    if( expectedResult ) {
        result = codeGenCast( codeGen, result, expectedResult, *op.op, true );
    }

    return result;
}

static AST::Expression codeGenSafeCast(
        LookupContext &ctx, FunctionGen *codeGen, ExpectedType expectedResult,
        const NonTerminals::Expression::BinaryOperator &op)
{
    AST::Type expectedType( op.operand1.get() );
    expectedType.symbolsPass2( &ctx );
    ExpectedType castExpectedResult( expectedType.getType(), true );

    // First code-gen the expression with mandatory result in our cast type
    AST::Expression interimResult = codeGenExpression( ctx, codeGen, castExpectedResult, op.operand2.get() );

    // Next, implicit cast it to our expected result
    return codeGenCast( codeGen, interimResult, expectedResult, *op.op, true );
}

static const std::unordered_map<
    Tokens,
    AST::Expression (*)(
            LookupContext &, FunctionGen *, ExpectedType, const NonTerminals::Expression::BinaryOperator &
    )
> binaryOperatorsMap{
    { Tokens::OP_PLUS, codeGenBinaryPlus },
    { Tokens::OP_MINUS, codeGenBinaryMinus },
    { Tokens::RESERVED_EXPECT, codeGenSafeCast },
};

AST::Expression codeGenBinaryOperator(
        LookupContext &ctx, FunctionGen *codeGen, ExpectedType expectedResult,
        const NonTerminals::Expression::BinaryOperator &op)
{
    ASSERT( op.op!=nullptr ) << "Operator codegen called with no operator";

    auto opHandler = binaryOperatorsMap.find( op.op->token );
    ASSERT( opHandler!=binaryOperatorsMap.end() ) <<
            "Code generation for unimplemented binary operator "<<op.op->token<<" at "<<op.op->line<<":"<<op.op->col<<
            "\n";

    return opHandler->second( ctx, codeGen, expectedResult, op );
}

} // end namespace CodeGen
