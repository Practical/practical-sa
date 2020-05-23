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

#include "ast/cast_op.h"
#include "ast/expected_result.h"
#include "ast/lookup_context.h"
#include "ast/value_range_base.h"
#include <practical-sa.h>

namespace AST::ExpressionImpl {

class Base {
private:
    std::unique_ptr<CastOperation> castOp;

protected:
    struct ExpressionMetadata {
        PracticalSemanticAnalyzer::StaticType::CPtr type;
        ValueRangeBase::CPtr valueRange;
    } metadata;

    class ExpressionTooExpensive : public std::exception {
    public:
        const char *what() const noexcept {
            return "Expression too expensive";
        }
    };

public:
    static constexpr unsigned NoWeightLimit = std::numeric_limits<unsigned>::max();

    static PracticalSemanticAnalyzer::ExpressionId allocateId();

    Base() = default;
    Base(Base &&rhs) = default;
    Base &operator=(Base &&rhs) = default;

    virtual ~Base() = 0;

    PracticalSemanticAnalyzer::StaticType::CPtr getType() const;

    ValueRangeBase::CPtr getValueRange() const {
        return metadata.valueRange;
    }

    void buildAST( LookupContext &lookupContext, ExpectedResult expectedResult, unsigned &weight, unsigned weightLimit );
    ExpressionId codeGen( PracticalSemanticAnalyzer::FunctionGen *functionGen );

protected:
    virtual void buildASTImpl(
            LookupContext &lookupContext, ExpectedResult expectedResult, unsigned &weight, unsigned weightLimit ) = 0;
    virtual ExpressionId codeGenImpl( PracticalSemanticAnalyzer::FunctionGen *functionGen ) = 0;
};

} // namespace AST::ExpressionImpl

#endif // AST_EXPRESSION_BASE_H
