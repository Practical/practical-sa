/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_POINTERS_H
#define AST_POINTERS_H

#include "ast/bool_value_range.h"

namespace AST {

class PointerValueRange final : public ValueRangeBase {
public:
    ValueRangeBase::CPtr pointedValueRange;
    BoolValueRange initialized;

    explicit PointerValueRange( ValueRangeBase::CPtr pointedRange ) :
        pointedValueRange( std::move( pointedRange ) ),
        initialized( false, true )
    {}

    explicit PointerValueRange( ValueRangeBase::CPtr pointedRange, const BoolValueRange &initialized ) :
        pointedValueRange( std::move(pointedRange) ),
        initialized( initialized.falseAllowed, initialized.trueAllowed )
    {}

    bool isLiteral() const override {
        if( ! initialized.isLiteral() )
            return false;

        if( initialized.falseAllowed )
            return true;

        ASSERT( pointedValueRange );
        return pointedValueRange->isLiteral();
    }
};

} // namespace AST

#endif // AST_POINTERS_H
