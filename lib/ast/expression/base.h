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

#include "ast/expression/expression_metadata.h"
#include "ast/cast_chain.h"
#include "ast/cast_op.h"
#include "ast/expected_result.h"
#include "ast/lookup_context.h"
#include "ast/weight.h"
#include <practical-sa.h>

namespace AST::ExpressionImpl {

class Base {
private:
    std::unique_ptr<CastChain> castChain;

protected:
    ExpressionMetadata metadata;

public:
    class ExpressionTooExpensive : public std::exception {
    public:
        const char *what() const noexcept {
            return "Expression too expensive";
        }
    };

    static constexpr Weight NoWeightLimit = Weight::max();

    static PracticalSemanticAnalyzer::ExpressionId allocateId();

    Base() = default;
    Base(Base &&rhs) = default;
    Base &operator=(Base &&rhs) = default;

    virtual ~Base() = 0;

    StaticTypeImpl::CPtr getType() const;

    ValueRangeBase::CPtr getValueRange() const {
        if( castChain )
            return castChain->getMetadata().valueRange;

        return metadata.valueRange;
    }

    void buildAST( LookupContext &lookupContext, ExpectedResult expectedResult, Weight &weight, Weight weightLimit );
    ExpressionId codeGen( PracticalSemanticAnalyzer::FunctionGen *functionGen ) const;

    virtual size_t getLine() const = 0;
    virtual size_t getCol() const = 0;

protected:
    virtual void buildASTImpl(
            LookupContext &lookupContext, ExpectedResult expectedResult, Weight &weight, Weight weightLimit ) = 0;
    virtual ExpressionId codeGenImpl( PracticalSemanticAnalyzer::FunctionGen *functionGen ) const = 0;
};

} // namespace AST::ExpressionImpl

#endif // AST_EXPRESSION_BASE_H
