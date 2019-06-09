/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef CODEGEN_OPERATORS_H
#define CODEGEN_OPERATORS_H

#include "ast_nodes.h"

Expression codeGenBinaryOperator(
        AST::CompoundExpression *astExpression, FunctionGen *codeGen, ExpectedType expectedResult,
        const NonTerminals::Expression::BinaryOperator &op);

#endif // CODEGEN_OPERATORS_H
