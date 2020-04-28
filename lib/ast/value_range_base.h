/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef AST_VALUE_RANGE_BASE_H
#define AST_VALUE_RANGE_BASE_H

#include "nocopy.h"

#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <boost/smart_ptr/intrusive_ptr.hpp>

namespace AST {

class ValueRangeBase : private NoCopy, public boost::intrusive_ref_counter<ValueRangeBase, boost::thread_unsafe_counter>
{
public:
    virtual ~ValueRangeBase() {}

    virtual bool isLiteral() const = 0;

    using CPtr = boost::intrusive_ptr<const ValueRangeBase>;
};

} // namespace AST

#endif // AST_VALUE_RANGE_BASE_H
