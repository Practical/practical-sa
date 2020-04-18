/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_EXPRESSION_BASE_H
#define AST_EXPRESSION_BASE_H

#include "ast/expected_result.h"
#include "ast/lookup_context.h"
#include <practical-sa.h>

namespace AST::ExpressionImpl {

class Base {
protected:
    PracticalSemanticAnalyzer::StaticType::CPtr type;

public:
    static PracticalSemanticAnalyzer::ExpressionId allocateId();

    virtual ~Base() = 0;

    PracticalSemanticAnalyzer::StaticType::CPtr getType() const {
        return type;
    }

    virtual void buildAST( LookupContext &lookupContext, ExpectedResult expectedResult ) = 0;
    virtual ExpressionId codeGen( PracticalSemanticAnalyzer::FunctionGen *functionGen ) = 0;
};

} // namespace AST::ExpressionImpl

#endif // AST_EXPRESSION_BASE_H
