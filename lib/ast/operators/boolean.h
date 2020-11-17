/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_OPERATORS_BOOLEAN_H
#define AST_OPERATORS_BOOLEAN_H

#include "ast/expression.h"

#include <practical/practical.h>

namespace AST::Operators {

// ==
ExpressionId equalsCodegenInt(
        Slice<const Expression>, const LookupContext::Function::Definition *, PracticalSemanticAnalyzer::FunctionGen *);
ValueRangeBase::CPtr equalsVrpUnsigned(StaticTypeImpl::CPtr functType, Slice<ValueRangeBase::CPtr> inputRanges);
ValueRangeBase::CPtr equalsVrpSigned(StaticTypeImpl::CPtr functType, Slice<ValueRangeBase::CPtr> inputRanges);
// !=
ExpressionId notEqualsCodegenInt(
        Slice<const Expression>, const LookupContext::Function::Definition *, PracticalSemanticAnalyzer::FunctionGen *);
ValueRangeBase::CPtr notEqualsVrpUnsigned(StaticTypeImpl::CPtr functType, Slice<ValueRangeBase::CPtr> inputRanges);
ValueRangeBase::CPtr notEqualsVrpSigned(StaticTypeImpl::CPtr functType, Slice<ValueRangeBase::CPtr> inputRanges);

// <
ExpressionId lessThanCodegenUInt(
        Slice<const Expression>, const LookupContext::Function::Definition *, PracticalSemanticAnalyzer::FunctionGen *);
ValueRangeBase::CPtr lessThanVrpUnsigned(StaticTypeImpl::CPtr functType, Slice<ValueRangeBase::CPtr> inputRanges);

ExpressionId lessThanCodegenSInt(
        Slice<const Expression>, const LookupContext::Function::Definition *, PracticalSemanticAnalyzer::FunctionGen *);
ValueRangeBase::CPtr lessThanVrpSigned(StaticTypeImpl::CPtr functType, Slice<ValueRangeBase::CPtr> inputRanges);

// <=
ExpressionId lessThanOrEqualsCodegenUInt(
        Slice<const Expression>, const LookupContext::Function::Definition *, PracticalSemanticAnalyzer::FunctionGen *);
ValueRangeBase::CPtr lessThanOrEqualsVrpUnsigned(StaticTypeImpl::CPtr functType, Slice<ValueRangeBase::CPtr> inputRanges);

ExpressionId lessThanOrEqualsCodegenSInt(
        Slice<const Expression>, const LookupContext::Function::Definition *, PracticalSemanticAnalyzer::FunctionGen *);
ValueRangeBase::CPtr lessThanOrEqualsVrpSigned(StaticTypeImpl::CPtr functType, Slice<ValueRangeBase::CPtr> inputRanges);

// >
ExpressionId greaterThenCodegenUInt(
        Slice<const Expression>, const LookupContext::Function::Definition *, PracticalSemanticAnalyzer::FunctionGen *);
ValueRangeBase::CPtr greaterThenVrpUnsigned(StaticTypeImpl::CPtr functType, Slice<ValueRangeBase::CPtr> inputRanges);

ExpressionId greaterThenCodegenSInt(
        Slice<const Expression>, const LookupContext::Function::Definition *, PracticalSemanticAnalyzer::FunctionGen *);
ValueRangeBase::CPtr greaterThenVrpSigned(StaticTypeImpl::CPtr functType, Slice<ValueRangeBase::CPtr> inputRanges);

// >=
ExpressionId greaterThenOrEqualsCodegenUInt(
        Slice<const Expression>, const LookupContext::Function::Definition *, PracticalSemanticAnalyzer::FunctionGen *);
ValueRangeBase::CPtr greaterThenOrEqualsVrpUnsigned(StaticTypeImpl::CPtr functType, Slice<ValueRangeBase::CPtr> inputRanges);

ExpressionId greaterThenOrEqualsCodegenSInt(
        Slice<const Expression>, const LookupContext::Function::Definition *, PracticalSemanticAnalyzer::FunctionGen *);
ValueRangeBase::CPtr greaterThenOrEqualsVrpSigned(StaticTypeImpl::CPtr functType, Slice<ValueRangeBase::CPtr> inputRanges);


// &&
ExpressionId logicalAnd(
        Slice<const Expression>, const LookupContext::Function::Definition *, PracticalSemanticAnalyzer::FunctionGen *);
ValueRangeBase::CPtr logicalAndVrp(StaticTypeImpl::CPtr functType, Slice<ValueRangeBase::CPtr> inputRanges);

// ||
ExpressionId logicalOr(
        Slice<const Expression>, const LookupContext::Function::Definition *, PracticalSemanticAnalyzer::FunctionGen *);
ValueRangeBase::CPtr logicalOrVrp(StaticTypeImpl::CPtr functType, Slice<ValueRangeBase::CPtr> inputRanges);


// !
ExpressionId logicalNot(
        Slice<const Expression>, const LookupContext::Function::Definition *, PracticalSemanticAnalyzer::FunctionGen *);
ValueRangeBase::CPtr logicalNotVrp(StaticTypeImpl::CPtr functType, Slice<ValueRangeBase::CPtr> inputRanges);

} // namespace AST::Operators

#endif // AST_OPERATORS_BOOLEAN_H
