/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_EXPRESSION_H
#define AST_EXPRESSION_H

#include "ast/expected_result.h"
#include "ast/expression/base.h"
#include "parser.h"

#include "ast/lookup_context.h"

#include <practical-sa.h>

namespace AST {

class Expression final : public ExpressionImpl::Base {
    const NonTerminals::Expression &parserExpression;
    std::unique_ptr< ExpressionImpl::Base > actualExpression;

public:
    explicit Expression( const NonTerminals::Expression &parserExpression );

    template<typename T>
    const T *tryGetActualExpression() const {
        return dynamic_cast<const T *>(actualExpression.get());
    }

protected:
    void buildASTImpl( LookupContext &lookupContext, ExpectedResult expectedResult ) override;
    ExpressionId codeGenImpl( PracticalSemanticAnalyzer::FunctionGen *functionGen ) override;
};

} // namespace AST

#endif // AST_EXPRESSION_H
