/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2019 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "function_call.h"

#include "ast/expression/identifier.h"

#include <practical/errors.h>

using namespace PracticalSemanticAnalyzer;

namespace AST::ExpressionImpl {

FunctionCall::FunctionCall( const NonTerminals::Expression::FunctionCall &parserFunctionCall ) :
    parserFunctionCall( parserFunctionCall )
{
}

SourceLocation FunctionCall::getLocation() const {
    return parserFunctionCall.op->location;
}

// protected methods
void FunctionCall::buildASTImpl(
        LookupContext &lookupContext, ExpectedResult expectedResult, Weight &weight, Weight weightLimit )
{
    functionId.emplace( *parserFunctionCall.expression );
    functionId->buildAST( lookupContext, ExpectedResult(), weight, weightLimit );

    const Identifier *identifier = functionId->tryGetActualExpression<Identifier>();
    ASSERT(identifier)<<"TODO calling function through generic pointer expression not yet implemented";

    struct Visitor {
        FunctionCall *_this;
        LookupContext &lookupContext;
        ExpectedResult &expectedResult;
        Weight &weight;
        const Weight weightLimit;

        void operator()( const LookupContext::Variable &var ) {
            ABORT()<<"TODO calling function through a variable not yet implemented";
        }

        void operator()( const LookupContext::Function &function ) {
            size_t numArguments = _this->parserFunctionCall.arguments.arguments.size();
            const NonTerminals::Expression *arguments[numArguments];

            for( unsigned i=0; i<numArguments; ++i ) {
                arguments[i] = &_this->parserFunctionCall.arguments.arguments[i];
            }

            _this->resolver.resolveOverloads(
                    lookupContext, expectedResult, function.overloads,
                    weight, weightLimit,
                    _this->metadata, Slice(arguments, numArguments), _this->parserFunctionCall.op );
        }
    };

    std::visit(
            Visitor{
                ._this=this, .lookupContext=lookupContext, .expectedResult=expectedResult,
                .weight=weight, .weightLimit=weightLimit,
            },
            *identifier->getCtxIdentifier()
        );
}

ExpressionId FunctionCall::codeGenImpl( PracticalSemanticAnalyzer::FunctionGen *functionGen ) const {
    return resolver.codeGen( functionGen );
}

} // namespace AST::ExpressionImpl
