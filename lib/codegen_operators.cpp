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
#include "dummy_codegen_impl.h"
#include "tokenizer.h"

#include "practical-errors.h"

using Tokenizer::Tokens;

Expression codeGenUnaryPlus(
        AST::CompoundExpression *astExpression, FunctionGen *codeGen, ExpectedType expectedResult,
        const NonTerminals::Expression::UnaryOperator &op)
{
    return astExpression->codeGenExpression( codeGen, expectedResult, op.operand.get() );
}

static const std::unordered_map<
    Tokens,
    Expression (*)(
            AST::CompoundExpression *, FunctionGen *, ExpectedType, const NonTerminals::Expression::UnaryOperator &
    )
> unaryOperatorsMap {
    { Tokens::OP_PLUS, codeGenUnaryPlus },
};

Expression codeGenUnaryOperator(
        AST::CompoundExpression *astExpression, FunctionGen *codeGen, ExpectedType expectedResult,
        const NonTerminals::Expression::UnaryOperator &op)
{
    ASSERT( op.op!=nullptr ) << "Operator codegen called with no operator";

    auto opHandler = unaryOperatorsMap.find( op.op->token );
    ASSERT( opHandler!=unaryOperatorsMap.end() ) <<
            "Code generation for unimplemented unary operator "<<op.op->token<<" at "<<op.op->line<<":"<<op.op->col<<
            "\n";

    return opHandler->second( astExpression, codeGen, expectedResult, op );
}

static StaticType::Ptr findCommonType(
        const PracticalSemanticAnalyzer::NamedType *type1,
        const PracticalSemanticAnalyzer::NamedType *type2 )
{
    if( type1==nullptr || type2==nullptr ) {
        return StaticType::Ptr();
    }

    if( type2->type()==NamedType::Type::UnsignedInteger ) {
        // Make sure that if either types is UnsignedInteger, that type1 is UnsignedInteger
        std::swap( type2, type1 );
    }

    if( type1->type()==NamedType::Type::UnsignedInteger ) {
        if( type2->type()==NamedType::Type::UnsignedInteger ) {
            return StaticType::allocate( (type1->size()>type2->size() ? type1 : type2)->id() );
        } else if( type2->type()==NamedType::Type::SignedInteger ) {
            // Unsigned and signed. This might get hairy
            if( type2->size() > type1->size() ) {
                return StaticType::allocate( type2->id() );
            }

            char retTypeName[10];
            snprintf(retTypeName, sizeof(retTypeName), "S%lu", type1->size()*2);
            auto retType = AST::AST::getGlobalCtx().lookupType(retTypeName);
            if( retType != nullptr ) {
                return StaticType::allocate( retType->id() );
            }
        }
    } else if( type1->type()==NamedType::Type::SignedInteger ) {
        if( type2->type()!=NamedType::Type::SignedInteger )
            return StaticType::Ptr();

        return StaticType::allocate( (type1->size()>type2->size() ? type1 : type2)->id() );
    }

    return StaticType::Ptr();
}

static ExpectedType findCommonType(
        const Tokenizer::Token *op, const LookupContext &ctx, const Expression &expr1, const Expression &expr2 )
{
    if( expr1.type == expr2.type )
        return ExpectedType( expr1.type );

    using PracticalSemanticAnalyzer::NamedType;
    auto type1 = ctx.lookupType( expr1.type->getId() );
    auto type2 = ctx.lookupType( expr2.type->getId() );

    auto retType = findCommonType( type1, type2 );
    if( !retType )
        throw PracticalSemanticAnalyzer::IncompatibleTypes( expr1.type, expr2.type, op->line, op->col );

    return ExpectedType( retType );
}

static void binaryOpFindCommonType(
        AST::CompoundExpression *astExpression, FunctionGen *codeGen, ExpectedType expectedResult,
        const NonTerminals::Expression::BinaryOperator &op, Expression &leftOperand, Expression &rightOperand
        )
{
    if( !expectedResult || !expectedResult.mandatory ) {
        // Calculate expression types as they are, and see what comes back

        // TODO: the recursion here has potential to re-calculate the same subtrees many times. Implement cache to speed
        // things up.
        leftOperand = astExpression->codeGenExpression( &dummyFunctionGen, expectedResult, op.operand1.get() );
        rightOperand = astExpression->codeGenExpression( &dummyFunctionGen, expectedResult, op.operand2.get() );
        expectedResult = findCommonType( op.op, astExpression->getContext(), leftOperand, rightOperand );
    }

    leftOperand = astExpression->codeGenExpression( codeGen, expectedResult, op.operand1.get() );
    rightOperand = astExpression->codeGenExpression( codeGen, expectedResult, op.operand2.get() );
}

static Expression codeGenBinaryPlus(
        AST::CompoundExpression *astExpression, FunctionGen *codeGen, ExpectedType expectedResult,
        const NonTerminals::Expression::BinaryOperator &op)
{
    Expression leftOperand, rightOperand;
    binaryOpFindCommonType(astExpression, codeGen, expectedResult, op, leftOperand, rightOperand);

    ASSERT( leftOperand.type == rightOperand.type );
    Expression result( StaticType::Ptr(leftOperand.type) );
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

static Expression codeGenBinaryMinus(
        AST::CompoundExpression *astExpression, FunctionGen *codeGen, ExpectedType expectedResult,
        const NonTerminals::Expression::BinaryOperator &op)
{
    ExpectedType operandExpectedType( expectedResult );
    operandExpectedType.mandatory = false;

    Expression leftOperand, rightOperand;
    binaryOpFindCommonType(astExpression, codeGen, operandExpectedType, op, leftOperand, rightOperand);

    ASSERT( leftOperand.type == rightOperand.type );
    Expression result( StaticType::Ptr(leftOperand.type) );
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

static const std::unordered_map<
    Tokens,
    Expression (*)(
            AST::CompoundExpression *, FunctionGen *, ExpectedType, const NonTerminals::Expression::BinaryOperator &
    )
> binaryOperatorsMap{
    { Tokens::OP_PLUS, codeGenBinaryPlus },
    { Tokens::OP_MINUS, codeGenBinaryMinus },
};

Expression codeGenBinaryOperator(
        AST::CompoundExpression *astExpression, FunctionGen *codeGen, ExpectedType expectedResult,
        const NonTerminals::Expression::BinaryOperator &op)
{
    ASSERT( op.op!=nullptr ) << "Operator codegen called with no operator";

    auto opHandler = binaryOperatorsMap.find( op.op->token );
    ASSERT( opHandler!=binaryOperatorsMap.end() ) <<
            "Code generation for unimplemented binary operator "<<op.op->token<<" at "<<op.op->line<<":"<<op.op->col<<
            "\n";

    return opHandler->second( astExpression, codeGen, expectedResult, op );
}