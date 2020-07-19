/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/operators/helper.h"

#include <practical-sa.h>

using namespace PracticalSemanticAnalyzer;

namespace AST::Operators {

const SignedIntValueRange *getSignedOverloadRange(
        const StaticTypeImpl::CPtr &funcType, Slice<const SignedIntValueRange *> argumentRanges )
{
    auto function = std::get<const StaticType::Function *>(funcType->getType());

    ASSERT( argumentRanges.size()==function->getNumArguments() );

    auto firstArgType = static_cast< const StaticTypeImpl * >(function->getArgumentType(0).get());
    auto firstArgRange = firstArgType->defaultRange();
    ASSERT( dynamic_cast< const SignedIntValueRange * >(firstArgRange.get()) != nullptr );

    return static_cast< const SignedIntValueRange * >(firstArgRange.get());
}

} // namespace AST::Operators
