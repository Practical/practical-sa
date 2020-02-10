/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "practical-sa.h"
#include "value_range.h"

#include <boost/intrusive_ptr.hpp>

#include <iostream>
#include <memory>

struct Expression : private NoCopy {
    PracticalSemanticAnalyzer::ExpressionId id;
    PracticalSemanticAnalyzer::StaticType::Ptr type;
    boost::intrusive_ptr<const ValueRange> valueRange;
    std::variant<std::monostate, PracticalSemanticAnalyzer::StaticType::Ptr> compileTimeValue;

    Expression();
    Expression( PracticalSemanticAnalyzer::StaticType::Ptr && type );
    Expression( Expression && that );

    Expression &operator=( Expression &&that );

    Expression duplicate() const;

    boost::intrusive_ptr<const ValueRange> getRange() const;
};

std::ostream &operator<<( std::ostream &out, const Expression &expr );

#endif // EXPRESSION_H
