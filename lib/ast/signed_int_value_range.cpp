/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/signed_int_value_range.h"

namespace AST {

boost::intrusive_ptr<SignedIntValueRange>
        SignedIntValueRange::allocate( LongEnoughIntSigned min, LongEnoughIntSigned max )
{
    boost::intrusive_ptr<SignedIntValueRange> ret = new SignedIntValueRange;
    ret->minimum = min;
    ret->maximum = max;

    return ret;
}

} // namespace AST
