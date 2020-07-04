/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_EXPRESSION_EXPRESSION_METADATA_H
#define AST_EXPRESSION_EXPRESSION_METADATA_H

#include "ast/static_type.h"
#include "ast/value_range_base.h"

namespace AST::ExpressionImpl {

    struct ExpressionMetadata {
        StaticTypeImpl::CPtr type;
        ValueRangeBase::CPtr valueRange;
    };

} // namespace AST::ExpressionImpl

#endif // AST_EXPRESSION_EXPRESSION_METADATA_H
