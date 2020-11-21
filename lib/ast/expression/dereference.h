/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_EXPRESSION_DEREFERENCE_H
#define AST_EXPRESSION_DEREFERENCE_H

#include "ast/expression.h"

namespace AST::ExpressionImpl {

class Dereference {
    Expression operand;

public:
    explicit Dereference( const NonTerminals::Expression &parserOperand );

    void buildASTImpl(
            LookupContext &lookupContext, ExpectedResult expectedResult, ExpressionMetadata &metadata,
            Weight &weight, Weight weightLimit
        );
    ExpressionId codeGen( PracticalSemanticAnalyzer::FunctionGen *functionGen ) const;
};

} // namespace AST::ExpressionImpl

#endif // AST_EXPRESSION_DEREFERENCE_H
