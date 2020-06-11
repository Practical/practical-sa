/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_SIGNED_INT_VALUE_RANGE_H
#define AST_SIGNED_INT_VALUE_RANGE_H

#include "ast/value_range_base.h"

#include <practical-sa.h>

#include <type_traits>
#include <limits>

namespace AST {

class SignedIntValueRange final : public ValueRangeBase {
public:
    LongEnoughIntSigned minimum, maximum;

    bool isLiteral() const override {
        return minimum==maximum;
    }

    static boost::intrusive_ptr<SignedIntValueRange> allocate( LongEnoughIntSigned min, LongEnoughIntSigned max );

    template<
            typename T,
            std::enable_if_t<
                std::is_signed_v<T> && std::is_integral_v<T>,
                int
            > = 0
    > static CPtr allocate() {
        return allocate( std::numeric_limits<T>::min(), std::numeric_limits<T>::max() );
    }
};

} // namespace AST

#endif // AST_SIGNED_INT_VALUE_RANGE_H
