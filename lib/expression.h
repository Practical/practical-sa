#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "practical-sa.h"

#include <boost/intrusive_ptr.hpp>

#include <iostream>

struct Expression : private NoCopy {
    PracticalSemanticAnalyzer::ExpressionId id;
    PracticalSemanticAnalyzer::StaticType::Ptr type;

    Expression();
    Expression( PracticalSemanticAnalyzer::StaticType::Ptr && type );
    Expression( Expression && that );

    Expression &operator=( Expression &&that );

    Expression duplicate() const;
};

std::ostream &operator<<( std::ostream &out, const Expression &expr );

#endif // EXPRESSION_H
