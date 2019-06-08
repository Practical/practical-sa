#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "full_range_int.h"
#include "practical-sa.h"

#include <boost/intrusive_ptr.hpp>

#include <iostream>
#include <memory>

struct ValueRange : private NoCopy, public boost::intrusive_ref_counter<ValueRange, boost::thread_unsafe_counter> {
    FullRangeInt minimum, maximum;

    ValueRange( FullRangeInt min, FullRangeInt max ) : minimum(min), maximum(max) {}

    static boost::intrusive_ptr<const ValueRange> allocate( FullRangeInt min, FullRangeInt max ) {
        return new ValueRange(min, max);
    }
};

inline std::ostream &operator<<( std::ostream &out, const ValueRange &range ) {
    out<<range.minimum<<" to "<<range.maximum;

    return out;
}

struct Expression : private NoCopy {
    PracticalSemanticAnalyzer::ExpressionId id;
    PracticalSemanticAnalyzer::StaticType::Ptr type;
    boost::intrusive_ptr<const ValueRange> valueRange;

    Expression();
    Expression( PracticalSemanticAnalyzer::StaticType::Ptr && type );
    Expression( Expression && that );

    Expression &operator=( Expression &&that );

    Expression duplicate() const;
};

std::ostream &operator<<( std::ostream &out, const Expression &expr );

#endif // EXPRESSION_H
