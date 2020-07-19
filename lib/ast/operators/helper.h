/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_OPERATORS_HELPER_H
#define AST_OPERATORS_HELPER_H

#include "ast/signed_int_value_range.h"
#include "ast/static_type.h"
#include "ast/unsigned_int_value_range.h"

namespace AST::Operators {

template<typename T>
std::vector<const T *> downcastValueRanges(Slice<ValueRangeBase::CPtr> baseRanges) {
    std::vector<const T *> ret;
    ret.reserve( baseRanges.size() );

    for( auto baseRange : baseRanges ) {
#ifndef NDEBUG
        const T *ptr = dynamic_cast<const T *>( baseRange.get() );
        ASSERT( ptr!=nullptr )<<"Downcast did from incorrect type";
#else
        const T *ptr = static_cast<const T *>( baseRange.get() );
#endif
        ret.emplace_back( ptr );
    }

    return ret;
}

const SignedIntValueRange *getSignedOverloadRange(
        const StaticTypeImpl::CPtr &funcType, Slice<const SignedIntValueRange *> argumentRanges );
const UnsignedIntValueRange *getUnsignedOverloadRange(
        const StaticTypeImpl::CPtr &funcType, Slice<const UnsignedIntValueRange *> argumentRanges );

} // namespace AST::Operators

#endif // AST_OPERATORS_HELPER_H
