/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef DUMMY_CODEGEN_IMPL_H
#define DUMMY_CODEGEN_IMPL_H

#include <practical-sa.h>

using PracticalSemanticAnalyzer::ArgumentDeclaration;
using PracticalSemanticAnalyzer::ExpressionId;
using PracticalSemanticAnalyzer::IdentifierId;
using PracticalSemanticAnalyzer::StaticType;

class DummyFunctionGen : public PracticalSemanticAnalyzer::FunctionGen {
    void functionEnter(
            IdentifierId id, String name, const StaticType& returnType, Slice<const ArgumentDeclaration> arguments,
            String file, size_t line, size_t col) override
    {}
    void functionLeave(IdentifierId id) override {}

    void returnValue(ExpressionId id) override {}
    void setLiteral(ExpressionId id, LongEnoughInt value, const StaticType &type) override {}

    // The ExpressionId refers to a pointer to the resulting allocated variable
    void allocateStackVar(ExpressionId id, const StaticType &type, String name) override {}
    void assign( ExpressionId lvalue, ExpressionId rvalue ) override {}
    void dereferencePointer( ExpressionId id, const StaticType &type, ExpressionId addr ) override {}

    void truncateInteger(
            ExpressionId id, ExpressionId source, const StaticType &sourceType, const StaticType &destType ) override {}
    void expandIntegerSigned(
            ExpressionId id, ExpressionId source, const StaticType &sourceType, const StaticType &destType ) override {}
    void expandIntegerUnsigned(
            ExpressionId id, ExpressionId source, const StaticType &sourceType, const StaticType &destType ) override {}
    void callFunctionDirect(
            ExpressionId id, String name, Slice<const ExpressionId> arguments, const StaticType &returnType ) override {}
};

extern DummyFunctionGen dummyFunctionGen;

#endif // DUMMY_CODEGEN_IMPL_H
