/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_ARRAYS_H
#define AST_ARRAYS_H

#include "ast/bool_value_range.h"

namespace AST {

class ArrayValueRange final : public ValueRangeBase {
public:
    // TODO switch to sparse array?
    std::vector<ValueRangeBase::CPtr> elementsValueRange;

    explicit ArrayValueRange( ValueRangeBase::CPtr elementsDefaultRange, size_t numElements ) :
        elementsValueRange( numElements, elementsDefaultRange )
    {}

    bool isLiteral() const override {
        for( auto &range : elementsValueRange ) {
            if( ! range->isLiteral() )
                return false;
        }

        return true;
    }
};

} // namespace AST

#endif // AST_ARRAYS_H
