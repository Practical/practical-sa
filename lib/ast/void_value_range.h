/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_VOID_VALUE_RANGE_H
#define AST_VOID_VALUE_RANGE_H

#include "ast/value_range_base.h"

namespace AST {

class VoidValueRange final : public ValueRangeBase {
public:
    bool isLiteral() const override {
        return true;
    }
};

} // namespace AST

#endif // AST_VOID_VALUE_RANGE_H
