/* This file is part of the Practical programming langauge. https://github.com/Practical/practical-sa
 *
 * To the extent header files enjoy copyright protection, this file is file is copyright (C) 2018-2020 by its authors
 * You can see the file's authors in the AUTHORS file in the project's home repository.
 *
 * This is available under the Boost license. The license's text is available under the LICENSE file in the project's
 * home directory.
 */
#include "ast/statement.h"

#include "ast/compound_statement.h"
#include "ast/expression.h"

namespace AST {

Statement::Statement( const NonTerminals::Statement &parserStatement ) : parserStatement(parserStatement)
{
}

void Statement::codeGen( LookupContext &lookupCtx, PracticalSemanticAnalyzer::FunctionGen *functionGen ) const {
    struct Visitor {
        LookupContext &lookupCtx;
        PracticalSemanticAnalyzer::FunctionGen *functionGen;

        void operator()( std::monostate mono ) {
            ABORT()<<"Statement is in monostate";
        }

        void operator()( const NonTerminals::Expression &parserExpression ) {
            Expression expression(parserExpression);
            unsigned weight=0;
            expression.buildAST(lookupCtx, ExpectedResult(), weight, Expression::NoWeightLimit);
            expression.codeGen(functionGen);
        }

        void operator()( const NonTerminals::VariableDefinition &varDef ) {
            ExpressionId varExpressionId = Expression::allocateId();
            auto varType = lookupCtx.lookupType( varDef.body.type );

            Expression initValue( *varDef.initValue );
            unsigned weight = 0;
            initValue.buildAST(lookupCtx, varType, weight, Expression::NoWeightLimit);
            ExpressionId initValueExpressionId = initValue.codeGen(functionGen);

            lookupCtx.addLocalVar( varDef.body.name.identifier, varType, varExpressionId );

            functionGen->allocateStackVar(varExpressionId, varType, varDef.body.name.identifier->text);

            functionGen->assign( varExpressionId, initValueExpressionId );
        }

        void operator()( const NonTerminals::Statement::ConditionalStatement &condition ) {
            ABORT()<<"TODO implement";
        }

        void operator()( const std::unique_ptr<NonTerminals::CompoundStatement> &parserCompound ) {
            CompoundStatement compound(*parserCompound, lookupCtx);
            compound.codeGen(functionGen);
        }
    };

    std::visit( Visitor{ .lookupCtx=lookupCtx, .functionGen=functionGen}, parserStatement.content );
}

} // namespace AST
