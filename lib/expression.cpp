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
