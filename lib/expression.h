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

#include "expected_type.h"
#include "lookup_context.h"
#include "parser.h"
#include "practical-sa.h"
#include "value_range.h"

#include <boost/intrusive_ptr.hpp>

#include <iostream>
#include <memory>

namespace AST {

class Literal {
};

struct Expression : private NoCopy {
    PracticalSemanticAnalyzer::ExpressionId id;
    PracticalSemanticAnalyzer::StaticType::Ptr type;
    boost::intrusive_ptr<const ValueRange> valueRange;
    std::variant<std::monostate, PracticalSemanticAnalyzer::StaticType::Ptr> compileTimeValue;
    const NonTerminals::Expression *ntExpression = nullptr;
    std::unique_ptr< std::variant<Literal> > astNode;

public:
    Expression();
    Expression( Expression && that );
    Expression &operator=( Expression &&that );

    explicit Expression( const NonTerminals::Expression &ntExpression );
    explicit Expression( PracticalSemanticAnalyzer::StaticType::Ptr && type );

    //Expression duplicate() const;

    void buildAst( LookupContext &ctx, ExpectedType expectedType );
    void codeGen( FunctionGen *codeGen );

    ExpressionId getId() const {
        return id;
    }

    boost::intrusive_ptr<const ValueRange> getValueRange() const {
        return valueRange;
    }

    PracticalSemanticAnalyzer::StaticType::Ptr getType() const {
        return type;
    }
};

std::ostream &operator<<( std::ostream &out, const Expression &expr );

class CompoundExpression : NoCopy {
    LookupContext ctx;
    const NonTerminals::CompoundExpression *parserExpression;

public:
    CompoundExpression(LookupContext *parentCtx, const NonTerminals::CompoundExpression *nt);

    void symbolsPass1();
    void symbolsPass2();

    void buildAst( ExpectedType expectedType );
    Expression codeGen( FunctionGen *codeGen );
};

} // End namespace AST

#endif // EXPRESSION_H
