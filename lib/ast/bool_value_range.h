/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_BOOL_VALUE_RANGE_H
#define AST_BOOL_VALUE_RANGE_H

#include "ast/value_range_base.h"

#include <type_traits>
#include <limits>

namespace AST {

class BoolValueRange final : public ValueRangeBase {
    bool falseAllowed = true;
    bool trueAllowed = true;

private:
    BoolValueRange( bool falseAllowed, bool trueAllowed ) :
        falseAllowed(falseAllowed), trueAllowed(trueAllowed)
    {}

public:
    bool isLiteral() const override {
        return (trueAllowed && !falseAllowed) || (!trueAllowed && falseAllowed);
    }

    static boost::intrusive_ptr<BoolValueRange> allocate( bool falseAllowed, bool trueAllowed ) {
        return new BoolValueRange( falseAllowed, trueAllowed );
    }

    static auto allocate() {
        return allocate( true, true );
    }
};

} // namespace AST

#endif // AST_BOOL_VALUE_RANGE_H
