/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/operators/plus.h"

namespace AST::Operators {

using namespace PracticalSemanticAnalyzer;

ExpressionId bPlusCodegen(
        Slice<const Expression> arguments,
        const LookupContext::Function::Definition *definition,
        PracticalSemanticAnalyzer::FunctionGen *functionGen)
{
    ASSERT(arguments.size()==2);

    std::array<ExpressionId,2> argumentIds;
    for( unsigned i=0; i<2; ++i ) {
        argumentIds[i] = arguments[i].codeGen(functionGen);
    }

    ExpressionId resultId = ExpressionImpl::Base::allocateId();
    functionGen->binaryOperatorPlus(
            resultId, argumentIds[0], argumentIds[1],
            std::get<const StaticType::Function *>(definition->type->getType())->getReturnType() );

    return resultId;
}

} // namespace AST::Operators
