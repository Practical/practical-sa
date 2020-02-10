#ifndef VALUE_RANGE_H
#define VALUE_RANGE_H

#include "full_range_int.h"

#include <boost/smart_ptr/intrusive_ref_counter.hpp>

struct ValueRange : private NoCopy, public boost::intrusive_ref_counter<ValueRange, boost::thread_unsafe_counter> {
    FullRangeInt minimum, maximum;

    ValueRange( FullRangeInt min, FullRangeInt max ) : minimum(min), maximum(max) {}

    static boost::intrusive_ptr<const ValueRange> allocate( FullRangeInt min, FullRangeInt max ) {
        return new ValueRange(min, max);
    }

    bool containedIn( const ValueRange &that ) const {
        return minimum>=that.minimum && maximum<=that.maximum;
    }
};

inline std::ostream &operator<<( std::ostream &out, const ValueRange &range ) {
    out<<range.minimum<<" to "<<range.maximum;

    return out;
}

#endif // VALUE_RANGE_H
