#ifndef CODEGEN_OPERATORS_H
#define CODEGEN_OPERATORS_H

#include "ast_nodes.h"

Expression codeGenBinaryOperator(
        AST::CompoundExpression *astExpression, FunctionGen *codeGen, ExpectedType expectedResult,
        const NonTerminals::Expression::BinaryOperator &op);

#endif // CODEGEN_OPERATORS_H
