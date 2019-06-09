/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * This file is file is copyright (C) 2018-2019 by its authors.
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "expression.h"

#include "asserts.h"

static PracticalSemanticAnalyzer::ExpressionId::Allocator<> expressionIdAllocator;

Expression::Expression() {
}

Expression::Expression( PracticalSemanticAnalyzer::StaticType::Ptr && type ) :
    id(expressionIdAllocator.allocate()),
    type(type)
{
}

Expression::Expression( Expression && that ) : id(that.id), type( std::move(that.type) ) {
    that.id=PracticalSemanticAnalyzer::ExpressionId();
}

Expression &Expression::operator=( Expression &&that ) {
    id = that.id;
    type = std::move( that.type );

    return *this;
}

Expression Expression::duplicate() const {
    Expression ret;
    ret.id = id;
    ret.type = type;

    return ret;
}

std::ostream &operator<<( std::ostream &out, const Expression &expr ) {
    out<<"Expression("<<expr.id<<": "<<expr.type<<")";

    return out;
}
