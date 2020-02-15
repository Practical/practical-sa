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

namespace AST {

static PracticalSemanticAnalyzer::ExpressionId::Allocator<> expressionIdAllocator;

Expression::Expression() {
}

Expression::Expression( Expression && that ) :
    id(that.id), type( std::move(that.type) ), valueRange( std::move(that.valueRange) )
{
    that.id=PracticalSemanticAnalyzer::ExpressionId();
}

Expression &Expression::operator=( Expression &&that ) {
    id = that.id;
    type = std::move( that.type );
    valueRange = std::move( that.valueRange );

    return *this;
}

Expression::Expression( const NonTerminals::Expression &ntExpression ) :
    id( expressionIdAllocator.allocate() ),
    ntExpression( &ntExpression )
{
}

#if 0
Expression Expression::duplicate() const {
    Expression ret;
    ret.id = id;
    ret.type = type;
    ret.valueRange = valueRange;

    return ret;
}
#endif

void Expression::buildAst( LookupContext &ctx, ExpectedType expectedType ) {
    ASSERT( ntExpression!=nullptr )<<"Cannot build AST for a non-organic expression";
    ASSERT( !astNode )<<"AST already built for expression";

    struct Visitor {
        void operator()( const std::unique_ptr<::NonTerminals::CompoundExpression> &compound ) {
            ABORT()<<"TODO implement";
        }

        void operator()( const ::NonTerminals::Literal &literal ) {
            ABORT()<<"TODO implement";
        }

        void operator()( const NonTerminals::Identifier &identifier ) {
            ABORT()<<"TODO implement";
        }

        void operator()( const NonTerminals::Expression::UnaryOperator &op ) {
            ABORT()<<"TODO implement";
        }

        void operator()( const NonTerminals::Expression::BinaryOperator &op ) {
            ABORT()<<"TODO implement";
        }

        void operator()( const NonTerminals::Expression::FunctionCall &func ) {
            ABORT()<<"TODO implement";
        }

        void operator()( const std::unique_ptr<NonTerminals::ConditionalExpression> &condition ) {
            ABORT()<<"TODO implement";
        }

        void operator()( const NonTerminals::Type &type ) {
            ABORT()<<"TODO implement";
        }
    };

    std::visit( Visitor(), ntExpression->value );
}

std::ostream &operator<<( std::ostream &out, const Expression &expr ) {
    out<<"Expression("<<expr.getId()<<": "<<expr.getType()<<")";

    return out;
}

} // End namespace AST
